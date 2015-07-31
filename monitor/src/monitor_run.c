/* Copyright (C) 2015 ANSSI

   This file is part of the Picon project.

   This file is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this file; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "call_stack.h"
#include "block_stack.h"

#include "monitor_load.h"
#include "monitor_run.h"

#include <unistd.h>









int monitor_run(const monitor_data data) {

  register monitor_state_t state;
  register module_id current_module = 0;
  register function_id current_function = 0;
  register block_stack_element curblock = 0;
  register const module_data *current_module_data = NULL;
  register const function_data *current_function_data = NULL;

  unsigned int error;
  call_stack calls;
  block_stack blocks;

  block_stack_element topblock = curblock;

  call_stack_element ncall;

#ifdef TIMEOUT
  fd_set fdrs;
#endif

  client_signal sig;
  const monitor_answer ok_resp = { .event = CFI_OK };

  LOG_DEBUG_MONITOR("entering monitor_run loop\n");

  state = SM_STATE_Ready;
  error = 0;

  if(call_stack_init(&calls)) {
    return 1;
  }
  if(block_stack_init(&blocks)) {
    call_stack_free(&calls);
    return 1;
  }

  MONITOR_SIGNAL_FDRS(data.fd_client_to_monitor, fdrs);

  MONITOR_SIGNAL_TIMEOUT(data.fd_client_to_monitor, fdrs, error);
  if(unlikely(error)) {
    LOG_ERROR_MONITOR("timeout waiting for client signal\n");
    goto monitor_run_exit;
  }
  READ_CLIENT_SIGNAL(data.fd_client_to_monitor, sig, error);

  while(1) {

    switch(sig.event) {

    case CFI_BEFORE_JUMP: /**/
      LOG_DEBUG_MONITOR("received BEFORE_JUMP (%u,%u,%u)\n", sig.module, sig.function, sig.block);
      if(unlikely((state != SM_STATE_InFunction) ||
                  (current_module != sig.module) ||
                  (current_function != sig.function) ||
                  (curblock != sig.block))) {
        error = 4;
        goto monitor_run_exit;
      }
      state = SM_STATE_ExpectJump;
      break;


    case CFI_AFTER_JUMP: /**/
      LOG_DEBUG_MONITOR("received AFTER_JUMP (%u,%u,%u)\n", sig.module, sig.function, sig.block);
      if(unlikely((state != SM_STATE_ExpectJump) ||
                  (current_module != sig.module) ||
                  (current_function != sig.function) ||
                  (sig.block >= current_function_data->nb_blocks) ||
                  (!(current_function_data->control_flow_graph[curblock][sig.block])))) {
        error = 4;
        goto monitor_run_exit;
      }
      if(unlikely(OPTION_sm_trace &&
                  (current_function_data->immediate_post_dominator[topblock] == sig.block))) {
        BLOCK_STACK_PUSH(blocks, topblock, error);
        topblock = sig.block;
      }
      curblock = sig.block;
      state = SM_STATE_InFunction;
      break;


    case CFI_CALL: /**/
      LOG_DEBUG_MONITOR("received CALL (%u,%u)\n", sig.module, sig.function);
      if(unlikely((state != SM_STATE_InFunction) ||
                  (sig.function >= current_module_data->nb_functions) ||
                  (!(current_module_data->call_graph[current_function][sig.function])))) {
        error = 4;
        goto monitor_run_exit;
      }
      if(unlikely(OPTION_sm_trace)) {
        BLOCK_STACK_PUSH(blocks, topblock, error);
      }
      BLOCK_STACK_PUSH(blocks, curblock, error);
      curblock = topblock = FUNCTION_ENTRY_BLOCK_ID;
      ncall = (call_stack_element)
        {
          .module = current_module,
          .function = current_function,
          .blocks_sp = BLOCK_STACK_TOP_PTR(blocks)
        };
      CALL_STACK_PUSH(calls, ncall, error);
      if(FUNCTION_IS_EXTERNAL(current_module_data->functions[sig.function])) {
        const module_function_ids some_reloc = current_module_data->relocations[sig.function];
        LOG_DEBUG_MONITOR("\trelocated CALL (%u,%u)\n", MODULE_FUNCTION_GET_MODULE(some_reloc), MODULE_FUNCTION_GET_FUNCTION(some_reloc));
        current_module = MODULE_FUNCTION_GET_MODULE(some_reloc);
        current_function = MODULE_FUNCTION_GET_FUNCTION(some_reloc);
        current_module_data = &(data.modules[current_module]);
      } else {
        current_function = sig.function;
      }
      current_function_data = &(current_module_data->functions[current_function]);
      state = SM_STATE_ExpectCall;
      break;


    case CFI_ENTER: /**/
      LOG_DEBUG_MONITOR("received ENTER (%u,%u)\n", sig.module, sig.function);
      if(unlikely(state == SM_STATE_Ready)) {
        if(unlikely(sig.module >= data.nb_modules)) {
          error = 6;
          goto monitor_run_exit;
        }
        current_module_data = &(data.modules[sig.module]);
        if(unlikely((sig.function >= current_module_data->nb_functions) ||
                    (!FUNCTION_IS_ENTRYPOINT(current_module_data->functions[sig.function])))) {
          error = 7;
          goto monitor_run_exit;
        } else {
          curblock = topblock = FUNCTION_ENTRY_BLOCK_ID;
          current_module = sig.module;
          current_function = sig.function;
          current_function_data = &(current_module_data->functions[sig.function]);
        }
      } else if(unlikely((state != SM_STATE_ExpectCall) ||
                         (current_module != sig.module) ||
                         (current_function != sig.function) ||
                         (curblock != FUNCTION_ENTRY_BLOCK_ID))) {
        error = 4;
        goto monitor_run_exit;
      }
      state = SM_STATE_InFunction;
      break;


    case CFI_EXIT: /**/
      LOG_DEBUG_MONITOR("received EXIT (%u,%u)\n", sig.module, sig.function);
      if(unlikely((state != SM_STATE_InFunction) ||
                  (current_module != sig.module) ||
                  (current_function != sig.function))) {
        error = 4;
        goto monitor_run_exit;
      }
      if(unlikely(CALL_STACK_IS_EMPTY(calls))) {
        if(unlikely(!FUNCTION_IS_ENTRYPOINT(current_module_data->functions[current_function]))) {
          error = 7;
          goto monitor_run_exit;
        }
        state = SM_STATE_Ready;
        /* simplify this case since unlikely */
        WRITE_MONITOR_ANSWER(data.fd_monitor_to_client, ok_resp, error);
        goto monitor_run_exit;
        /**/
      } else {
        CALL_STACK_POP(calls, ncall);
        current_module = ncall.module;
        current_function = ncall.function;
        current_module_data = &(data.modules[current_module]);
        current_function_data = &(current_module_data->functions[current_function]);
        BLOCK_STACK_RESTORE_TOP(blocks, ncall.blocks_sp);
        if(unlikely(BLOCK_STACK_IS_EMPTY(blocks))) {
          error = 7;
          goto monitor_run_exit;
        }
        BLOCK_STACK_POP(blocks, curblock);
        if(unlikely(OPTION_sm_trace)) {
          BLOCK_STACK_POP(blocks, topblock);
        }
        state = SM_STATE_ExpectReturn;
      }
      break;


    case CFI_RETURNED: /**/
      LOG_DEBUG_MONITOR("received RETURNED (%u,%u)\n", sig.module, sig.function);
      if(unlikely((state != SM_STATE_ExpectReturn) ||
                  (sig.function >= current_module_data->nb_functions) ||
                  (!(current_module_data->call_graph[current_function][sig.function])))) {
        error = 4;
        goto monitor_run_exit;
      }
      state = SM_STATE_InFunction;
      break;


    default:
      LOG_ERROR_MONITOR("malformed sig received\n");
      error = 5;
      goto monitor_run_exit;
    }

    WRITE_MONITOR_ANSWER(data.fd_monitor_to_client, ok_resp, error);

    MONITOR_SIGNAL_TIMEOUT(data.fd_client_to_monitor, fdrs, error);
    if(unlikely(error)) {
      LOG_ERROR_MONITOR("timeout waiting for client signal\n");
      goto monitor_run_exit;
    }
    READ_CLIENT_SIGNAL(data.fd_client_to_monitor, sig, error);
  }


 monitor_run_exit:
  if(error) {
    LOG_ERROR_MONITOR("run failed with error = %u\n", error);
    if(OPTION_sm_trace) {
      unsigned int err = 0;
      BLOCK_STACK_PUSH(blocks, curblock, err);
      ncall = (call_stack_element)
        {
          .module = current_module,
          .function = current_function,
          .blocks_sp = BLOCK_STACK_TOP_PTR(blocks)
        };
      CALL_STACK_PUSH(calls, ncall, err);

      call_stack_trace(&calls);
      block_stack_trace(&blocks);

      BLOCK_STACK_POP(blocks, curblock);
      CALL_STACK_POP(calls, ncall);
    }
  }

  call_stack_free(&calls);
  block_stack_free(&blocks);

  return error;
}
