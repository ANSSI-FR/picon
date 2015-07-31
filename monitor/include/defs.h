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

#ifndef __DEFS_H__
#define __DEFS_H__

#define _GNU_SOURCE

#include <sys/select.h>
#include <sys/time.h>

#include <picon/shared.h>
#include <picon/log.h>




#define ENV_SM_TRACE "CFI_TRACE"

#define ENV_SM_DUMP "CFI_DUMP"


#define CALL_STACK_EXTEND_SIZE 512

#define BLOCK_STACK_EXTEND_SIZE 4096




#define FUNCTION_ENTRY_BLOCK_ID (function_id)0



extern unsigned int OPTION_sm_trace;
extern unsigned int OPTION_sm_dump;
extern unsigned int OPTION_nb_preload_monitor_ok_answers;




#ifdef TIMEOUT

extern struct timespec OPTION_timeout_monitor_no_signal;

#define MONITOR_SIGNAL_FDRS(fd,fdrs)  \
  do {                                \
    FD_ZERO(&(fdrs));                 \
    FD_SET((fd), (&(fdrs)));          \
  } while(0)

#define MONITOR_SIGNAL_TIMEOUT(fd,fdrs,err)                             \
  do {                                                                  \
    if(unlikely(pselect((fd)+1, &(fdrs), NULL, NULL,                    \
                        &OPTION_timeout_monitor_no_signal, NULL) != 1)) { \
      (err) = 1;                                                        \
    }                                                                   \
  } while(0)

#else

#define MONITOR_SIGNAL_FDRS(fd,fdrs)

#define MONITOR_SIGNAL_TIMEOUT(fd,fdrs,err)

#endif











#endif
