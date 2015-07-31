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

#ifndef __BLOCK_STACK_H__
#define __BLOCK_STACK_H__

#include "defs.h"

typedef block_id block_stack_element;

typedef struct {
  block_stack_element *first;
  block_stack_element *last;
  block_stack_element *top;
} block_stack;



extern int block_stack_init(block_stack * const block_stack);

extern void block_stack_free(block_stack * const block_stack);

extern int block_stack_extend(block_stack * const block_stack);

extern void block_stack_trace(const block_stack * const call_stack);

#define BLOCK_STACK_IS_EMPTY(s) ((s).top < (s).first)

#define BLOCK_STACK_TOP_PTR(s) ((s).top)

#define BLOCK_STACK_RESTORE_TOP(s,t)            \
  do {                                          \
    (s).top = t;                                \
  } while(0)

#define BLOCK_STACK_PUSH(s,v,err)               \
  do {                                          \
    if(unlikely((s).top >= (s).last)) {         \
      (err) = block_stack_extend(&(s));         \
    }                                           \
    if(likely((err) == 0)) {                    \
      *(++((s).top)) = v;                       \
    }                                           \
  } while(0)

#define BLOCK_STACK_POP(s,v)                    \
  do {                                          \
    (v) = *((s).top--);                         \
  } while(0)

#define BLOCK_STACK_IS_NOT_COHERENT_OR_IS_EMPTY(s)      \
  (((s).top < (s).first) || ((s).last < (s).top))



#endif
