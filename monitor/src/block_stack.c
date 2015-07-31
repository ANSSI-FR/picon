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
#include "block_stack.h"

#include <stdlib.h>


int block_stack_init(block_stack * const block_stack) {
  const unsigned int msize = BLOCK_STACK_EXTEND_SIZE;

  if((block_stack->first = calloc(msize, sizeof(block_stack_element))) == NULL) {
    LOG_DEBUG_MONITOR("failed to allocate block_stack\n");
    return -1;
  }
  block_stack->last = block_stack->first + (msize - 1);
  block_stack->top = block_stack->first - 1; /* invalid ptr */

  return 0;
}

void block_stack_free(block_stack * const block_stack) {
  if(block_stack->first) {
    free(block_stack->first);
    block_stack->first = NULL;
    block_stack->top = NULL;
    block_stack->last = NULL;
  }
}

int block_stack_extend(block_stack * const block_stack) {
  ssize_t msize = (block_stack->last - block_stack->first + 1) + BLOCK_STACK_EXTEND_SIZE;
  const ssize_t rsize = msize * sizeof(block_stack_element);

  if((block_stack->first = realloc(block_stack->first, rsize)) == NULL) {
    LOG_DEBUG_MONITOR("failed to reallocate block_stack\n");
    return -1;
  }

  block_stack->last = block_stack->first + (msize - 1);

  return 0;
}


void block_stack_trace(const block_stack * const block_stack) {
  block_stack_element *ptr;

  if(BLOCK_STACK_IS_EMPTY(*block_stack)) {
    return;
  }

  ptr = block_stack->top;
  while(ptr >= block_stack->first) {
    LOG_TRACE_MONITOR("@0x%p  block=%6u\n", ptr, *ptr);
    --ptr;
  }

}
