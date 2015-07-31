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

#ifndef __MONITOR_RUN_H__
#define __MONITOR_RUN_H__


#include "call_stack.h"
#include "block_stack.h"
#include "monitor_load.h"

typedef enum {
  SM_STATE_Ready,
  SM_STATE_InFunction,
  SM_STATE_ExpectCall,
  SM_STATE_ExpectReturn,
  SM_STATE_ExpectJump,
} monitor_state_t;






extern int monitor_run(const monitor_data data);





#endif
