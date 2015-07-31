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

#ifndef __MONITOR_LOAD_H__
#define __MONITOR_LOAD_H__


#include "defs.h"

#include <inttypes.h>
#include <unistd.h>




typedef struct {
  char *name;
  fct_flag_t flags;
  unsigned int nb_blocks;
  uint8_t **control_flow_graph; /* block_id -> block_id -> bool */
  block_id *immediate_post_dominator; /* block_id -> block_id */
} function_data;


#define FUNCTION_IS_ENTRYPOINT_MAIN(fdata) ((fdata).flags == FCT_MAIN)
#define FUNCTION_IS_ENTRYPOINT_INIT(fdata) ((fdata).flags == FCT_INIT)
#define FUNCTION_IS_ENTRYPOINT_FINI(fdata) ((fdata).flags == FCT_FINI)
#define FUNCTION_IS_ENTRYPOINT(fdata) (FUNCTION_IS_ENTRYPOINT_MAIN((fdata)) || \
                                       FUNCTION_IS_ENTRYPOINT_INIT((fdata)) || \
                                       FUNCTION_IS_ENTRYPOINT_FINI((fdata)))

#define FUNCTION_IS_EXTERNAL(fdata) ((fdata).flags == FCT_EXTERNAL)


typedef struct {
  unsigned int nb_functions;
  unsigned int nb_entrypoint_inits;
  unsigned int nb_entrypoint_finis;
  uint8_t **call_graph; /* local_fun_id -> local_fun_id -> bool */
  module_function_ids *relocations; /* local_fun_id -> (module_id,local_fun_id) */
  function_data *functions; /* local_fun_id -> function_data */
} module_data;


typedef struct {
  pid_t client_pid;

  int fd_client_to_monitor;
  int fd_monitor_to_client;
  int fd_loading_to_monitor;
  int fd_monitor_to_loading;

  unsigned int nb_modules;
  module_data *modules;
} monitor_data;


extern int monitor_load(const pid_t client_pid,
                        const int fd_client_to_monitor,
                        const int fd_monitor_to_client,
                        const int fd_loading_to_monitor,
                        const int fd_monitor_to_loading,
                        monitor_data * const data);

extern void monitor_data_free(monitor_data * const data);


#endif
