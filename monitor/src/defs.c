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



unsigned int OPTION_sm_trace = 0;

unsigned int OPTION_sm_dump = 0;

unsigned int OPTION_nb_preload_monitor_ok_answers =
#ifdef NB_PRELOADED_MONITOR_OK_ANSWERS
  NB_PRELOADED_MONITOR_OK_ANSWERS
#else
  0
#endif
  ;



#ifdef TIMEOUT


struct timespec OPTION_timeout_monitor_no_signal =
  {
    .tv_sec =
#ifdef TIMEOUT_SEC_MONITOR_NO_SIGNAL
    TIMEOUT_SEC_MONITOR_NO_SIGNAL
#else
    0
#endif
    ,

    .tv_nsec =
#ifdef TIMEOUT_NANOSEC_MONITOR_NO_SIGNAL
    TIMEOUT_NANOSEC_MONITOR_NO_SIGNAL
#else
    0
#endif
    ,
  };


#endif
