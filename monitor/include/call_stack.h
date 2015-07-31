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

#ifndef __CALL_STACK_H__
#define __CALL_STACK_H__


#include "block_stack.h"



typedef struct {
  module_id module;
  function_id function;
  block_stack_element *blocks_sp;
} call_stack_element;

typedef struct {
  call_stack_element *first;
  call_stack_element *last;
  call_stack_element *top;
} call_stack;



extern int call_stack_init(call_stack * const call_stack);

extern void call_stack_free(call_stack * const call_stack);

extern int call_stack_extend(call_stack * const call_stack);

extern void call_stack_trace(const call_stack * const call_stack);

#define CALL_STACK_IS_EMPTY(s) ((s).top < (s).first)

#define CALL_STACK_PUSH(s,v,err)                \
  do {                                          \
    if(unlikely((s).top >= (s).last)) {         \
      (err) = call_stack_extend(&(s));          \
    }                                           \
    if(likely((err) == 0)) {                    \
      *(++((s).top)) = v;                       \
    }                                           \
  } while(0)


#define CALL_STACK_POP(s,v)                     \
  do {                                          \
    (v) = *((s).top--);                         \
  } while(0)



#endif
