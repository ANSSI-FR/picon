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

#include <stdlib.h>



int call_stack_init(call_stack * const call_stack) {
  const unsigned int msize = CALL_STACK_EXTEND_SIZE;

  if((call_stack->first = calloc(msize,sizeof(call_stack_element))) == NULL) {
    LOG_DEBUG_MONITOR("failed to allocate call_stack\n");
    return -1;
  }
  call_stack->last = call_stack->first + (msize - 1);
  call_stack->top = call_stack->first - 1; /* invalid ptr */

  return 0;
}

void call_stack_free(call_stack * const call_stack) {
  if(call_stack->first) {
    free(call_stack->first);
    call_stack->first = NULL;
    call_stack->top = NULL;
    call_stack->last = NULL;
  }
}

int call_stack_extend(call_stack * const call_stack) {
  ssize_t msize = (call_stack->last - call_stack->first + 1) + CALL_STACK_EXTEND_SIZE;
  const ssize_t rsize = msize * sizeof(call_stack_element);

  if((call_stack->first = realloc(call_stack->first, rsize)) == NULL) {
    LOG_DEBUG_MONITOR("failed to reallocate call_stack\n");
    return -1;
  }

  call_stack->last = call_stack->first + (msize - 1);

  return 0;
}

void call_stack_trace(const call_stack * const call_stack) {
  call_stack_element *ptr;

  if(CALL_STACK_IS_EMPTY(*call_stack)) {
    return;
  }

  ptr = call_stack->top;
  while(ptr >= call_stack->first) {
    LOG_TRACE_MONITOR("module=%6u  function=%6u  blocks_sp=0x%p\n", ptr->module, ptr->function, ptr->blocks_sp);
    --ptr;
  }

}
