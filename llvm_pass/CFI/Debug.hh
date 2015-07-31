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

#ifndef DEBUG_H__
#define DEBUG_H__

#ifndef NDEBUG
#define ON_DEBUG(more_info, msg) \
         if (more_info) errs() << "DEBUG" << __FILE__ << "(" << __LINE__ << ") " << msg; \
         else  errs() << msg;
#else
#define ON_DEBUG(x, msg) do {} while (0)
#endif

#endif // !DEBUG_H__
