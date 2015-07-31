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

#include "monitor_dot.h"

void
monitor_call_graph(const module_data * const module) {
  unsigned int i, j;

  printf("digraph{\n");
  printf("\ttitle=\"Call graph of module\";\n");

  for(i = 0; i < module->nb_functions; ++i) {
    for(j = 0; j < module->nb_functions; ++j) {
      if(module->call_graph[i] &&
         module->call_graph[i][j]) {
        printf("\t%u -> %u;\n", i, j);
      }
    }
  }

  printf("}\n");
}


void monitor_control_flow_graph(const function_data * const function) {
  unsigned int i, j;

  printf("digraph{\n");
  printf("\ttitle=\"Control flow graph of function name=%s\";\n", function->name);

  for(i = 0; i < function->nb_blocks; ++i) {
    for(j = 0; j < function->nb_blocks; ++j) {
      if(function->control_flow_graph[i] &&
         function->control_flow_graph[i][j]) {
        printf("\t%u -> %u;\n", i, j);
      }
    }
  }

  printf("}\n");
}
