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
#include "monitor_dot.h"
#include "monitor_load.h"
#include "monitor_run.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>




static
int load_section_function_id(const msg_fct_l *const fct,
                             module_data * const data) {
  const msg_fct_t *ptr = NULL;
  unsigned int i;

  if(data->nb_functions) {
    LOG_ERROR_MONITOR("already processed section\n");
    return 1;
  }

  data->nb_functions = fct->num_fcts;
  if(((data->call_graph = calloc(data->nb_functions, sizeof(uint8_t*))) == NULL) ||
     ((data->relocations = calloc(data->nb_functions, sizeof(module_function_ids))) == NULL) ||
     ((data->functions = calloc(data->nb_functions, sizeof(function_data))) == NULL)) {
    LOG_ERROR_MONITOR("failed to allocate memory\n");
    return 2;
  }

  LOG_DEBUG_MONITOR("module has %u function references\n", data->nb_functions);

  ptr = fct->fcts;
  for(i = 0; i < data->nb_functions; ++i) {
    if(ptr->f_id >= data->nb_functions) {
      LOG_ERROR_MONITOR("invalid function_id\n");
      return 3;
    }
    function_data * const fd = &(data->functions[ptr->f_id]);
    fd->name = strdup(ptr->f_name);
    fd->flags = ptr->f_flags;
    fd->nb_blocks = 0;
    fd->control_flow_graph = NULL;
    fd->immediate_post_dominator = NULL;
    if(FUNCTION_IS_ENTRYPOINT_INIT(*fd)) {
      ++(data->nb_entrypoint_inits);
    }
    if(FUNCTION_IS_ENTRYPOINT_FINI(*fd)) {
      ++(data->nb_entrypoint_finis);
    }
    LOG_DEBUG_MONITOR("\tid=%6u main=%s init=%s fini=%s external=%s name=%s\n",
                      ptr->f_id,
                      FUNCTION_IS_ENTRYPOINT_MAIN(*fd) ? "YES" : "NO ",
                      FUNCTION_IS_ENTRYPOINT_INIT(*fd) ? "YES" : "NO ",
                      FUNCTION_IS_ENTRYPOINT_FINI(*fd) ? "YES" : "NO ",
                      FUNCTION_IS_EXTERNAL(*fd) ? "YES" : "NO ",
                      fd->name);
    ptr = (const msg_fct_t*)(1+strchr(ptr->f_name,'\0'));
  }

  return 0;
}

static
int load_section_function_transition(const msg_fct_trans_l * const trans,
                                     module_data * const data) {
  const msg_fct_trans_t *ptr = NULL;
  unsigned int i;

  if(!data->call_graph) {
    LOG_ERROR_MONITOR("call_graph not allocated\n");
    return 1;
  }

  if(data->nb_functions <= 0) {
    LOG_ERROR_MONITOR("missing functions count\n");
    return 1;
  }

  ptr = trans->fct_trans;
  for(i = 0; i < trans->num_fct_trans; ++i) {
    unsigned int j;
    const function_id * cptr = NULL;

    if(ptr->f_id >= data->nb_functions) {
      LOG_ERROR_MONITOR("invalid function_id\n");
      return 3;
    }

    if(data->call_graph[ptr->f_id]) {
      LOG_ERROR_MONITOR("already processed function id=%u\n", ptr->f_id);
      return 3;
    }

    if((data->call_graph[ptr->f_id] = calloc(data->nb_functions, sizeof(uint8_t))) == NULL) {
      LOG_ERROR_MONITOR("failed to allocate memory\n");
      return 2;
    }
    cptr = ptr->f_calls;
    for(j = 0; j < ptr->f_num_calls; ++j) {
      if(*cptr >= data->nb_functions) {
        LOG_ERROR_MONITOR("invalid function_id = %u\n", *cptr);
        return 3;
      }
      LOG_DEBUG_MONITOR("\tcall from->to = %6u -> %6u\n", ptr->f_id, *cptr);
      data->call_graph[ptr->f_id][*cptr] = 1;
      ++cptr;
    }

    ptr = (const msg_fct_trans_t *)((const char*)ptr + sizeof(msg_fct_trans_t) + ptr->f_num_calls*sizeof(uint32_t));
  }

  return 0;
}

static
int load_section_block_transition(const msg_block_trans_l * const trans,
                                  module_data * const data) {
  const msg_block_trans_t *ptr = NULL;
  function_data *fdata = NULL;
  unsigned int i;

  if(!data->functions) {
    LOG_ERROR_MONITOR("functions not allocated\n");
    return 1;
  }

  if(data->nb_functions <= 0) {
    LOG_ERROR_MONITOR("missing functions count\n");
    return 2;
  }

  if(trans->f_id >= data->nb_functions) {
    LOG_ERROR_MONITOR("invalid function_id\n");
    return 3;
  }

  fdata = &(data->functions[trans->f_id]);

  if(fdata->nb_blocks) {
    LOG_ERROR_MONITOR("already processed function id=%u\n", trans->f_id);
    return 4;
  }

  if(trans->num_block <= 0) {
    LOG_ERROR_MONITOR("missing block count\n");
    return 5;
  }

  fdata->nb_blocks = trans->num_block;
  if(((fdata->control_flow_graph = calloc(fdata->nb_blocks, sizeof(uint8_t*))) == NULL) ||
     ((fdata->immediate_post_dominator = calloc(fdata->nb_blocks, sizeof(block_id))) == NULL)) {
    LOG_ERROR_MONITOR("failed to allocate memory\n");
    return 6;
  }

  ptr = trans->block_trans;
  for(i = 0; i < trans->num_block_trans; ++i) {
    unsigned int j;
    const block_id * cptr = NULL;

    if(ptr->b_id >= fdata->nb_blocks) {
      LOG_ERROR_MONITOR("invalid block_id %u\n", ptr->b_id);
      return 7;
    }

    if(fdata->control_flow_graph[ptr->b_id] == NULL) {
      if((fdata->control_flow_graph[ptr->b_id] = calloc(fdata->nb_blocks, sizeof(uint8_t))) == NULL) {
        LOG_ERROR_MONITOR("failed to allocate memory\n");
        return 9;
      }
    }

    cptr = ptr->b_succ;
    for(j = 0; j < ptr->b_num_succ; ++j) {
      if(*cptr >= fdata->nb_blocks) {
        LOG_ERROR_MONITOR("invalid block_id %u (nb_blocks = %u)\n", *cptr, fdata->nb_blocks);
        return 10;
      }
      LOG_DEBUG_MONITOR("\tjump from->to = %6u -> %6u\n", ptr->b_id, *cptr);
      fdata->control_flow_graph[ptr->b_id][*cptr] = 1;
      ++cptr;
    }

    ptr = (const msg_block_trans_t*)((const char*)ptr + sizeof(msg_block_trans_t) + ptr->b_num_succ*(sizeof(uint32_t)));
  }

  return 0;
}

static
int load_section_block_ipd(const msg_block_ipd_l * const ipd,
                           module_data * const data) {
  const msg_block_ipd_t *ptr = NULL;
  unsigned int i;

  if(!data->functions) {
    LOG_ERROR_MONITOR("functions not allocated\n");
    return 1;
  }

  ptr = ipd->block_ipd;
  for(i = 0; i < ipd->num_ipd; ++i) {
    const function_data *fdata = NULL;

    if(ptr->f_id >= data->nb_functions) {
      LOG_ERROR_MONITOR("invalid function_id\n");
      return 2;
    }

    fdata = &(data->functions[ptr->f_id]);

    if(!(fdata->immediate_post_dominator)) {
      LOG_ERROR_MONITOR("immediate_post_dominator not allocated\n");
      return 4;
    }

    if(ptr->b_id >= fdata->nb_blocks) {
      LOG_ERROR_MONITOR("invalid block_id\n");
      return 5;
    }

    LOG_DEBUG_MONITOR("\tfunction=%6u block=%6u ipd=%6u\n", ptr->f_id, ptr->b_id, ptr->b_ipd_id);
    fdata->immediate_post_dominator[ptr->b_id] = ptr->b_ipd_id;

    ++ptr;
  }


  return 0;
}

int monitor_load_module(monitor_data * const mon) {
  loading_packet pkt;
  monitor_to_loading_packet mpkt;
  module_data *modata = NULL;
  int err = 0;
  int terminated = 0;

  LOG_DEBUG_MONITOR("waiting for new module\n");

  pkt.size = 0;
  pkt.value = NULL;

  READ_LOADING_PACKET(mon->fd_loading_to_monitor, pkt, err);
  if(err) {
    LOG_ERROR_MONITOR("failed to read loading packet (err = %u)\n", err);
  } else {
    switch(pkt.event) {
    case CFI_LOADING_MODULE_BEGIN:
      LOG_DEBUG_MONITOR("received CFI_LOADING_MODULE_BEGIN\n");
      if(mon->nb_modules) {
        ++(mon->nb_modules);
        mon->modules = realloc(mon->modules, sizeof(module_data) * mon->nb_modules);
      } else {
        mon->nb_modules = 1;
        mon->modules = calloc(1, sizeof(module_data));
      }
      if(mon->modules == NULL) {
        LOG_ERROR_MONITOR("failed to allocate memory\n");
        err = 10;
      } else {
        modata = &(mon->modules[mon->nb_modules - 1]);
        modata->nb_functions = 0;
        modata->nb_entrypoint_inits = 0;
        modata->nb_entrypoint_finis = 0;
        modata->call_graph = NULL;
        modata->relocations = NULL;
        modata->functions = NULL;

        mpkt.id = mon->nb_modules - 1;
        WRITE_MONITOR_TO_LOADING_PACKET(mon->fd_monitor_to_loading, mpkt, err);
      }
      break;

    case CFI_LOADING_TERMINATED:
      LOG_DEBUG_MONITOR("received CFI_LOADING_TERMINATED\n");
      terminated = 1;
      break;

    default:
      LOG_ERROR_MONITOR("unexpected loading event\n");
      err = 5;
      break;
    }
  }



  while((err == 0) &&
        (terminated == 0) &&
        (pkt.event != CFI_LOADING_MODULE_END)) {

    FREE_LOADING_PACKET(pkt);

    READ_LOADING_PACKET(mon->fd_loading_to_monitor, pkt, err);

    if(err) {
      LOG_ERROR_MONITOR("failed to read loading packet\n");
      err = 4;

    } else {

      switch(pkt.event) {
      case CFI_LOADING_SECTION_FUNCTION_ID:
        LOG_DEBUG_MONITOR("received CFI_LOADING_SECTION_FUNCTION_ID\n");
        err = load_section_function_id(&(pkt.value->fct), modata);
        break;

      case CFI_LOADING_SECTION_FUNCTION_TRANSITION:
        LOG_DEBUG_MONITOR("received CFI_LOADING_SECTION_FUNCTION_TRANSITION\n");
        err = load_section_function_transition(&(pkt.value->fct_trans), modata);
        break;

      case CFI_LOADING_SECTION_BLOCK_TRANSITION:
        LOG_DEBUG_MONITOR("received CFI_LOADING_SECTION_BLOCK_TRANSITION\n");
        err = load_section_block_transition(&(pkt.value->block_trans), modata);
        break;

      case CFI_LOADING_SECTION_BLOCK_IPD:
        LOG_DEBUG_MONITOR("received CFI_LOADING_SECTION_BLOCK_IPD\n");
        err = load_section_block_ipd(&(pkt.value->block_ipd), modata);
        break;

      case CFI_LOADING_MODULE_END:
        LOG_DEBUG_MONITOR("received CFI_LOADING_MODULE_END\n");
        if(OPTION_sm_dump) {
          unsigned int i;
          monitor_call_graph(modata);
          for(i = 0; i < modata->nb_functions; ++i) {
            monitor_control_flow_graph(&(modata->functions[i]));
          }
        }
        break;

      default:
        LOG_ERROR_MONITOR("unexpected loading event\n");
        err = 5;
        break;
      }
    }
  }

  FREE_LOADING_PACKET(pkt);

  if(terminated) return 1;
  if(err) return -1;

  return 0;
}



static
int compute_relocations_of_module(monitor_data * const data,
                                  const module_id mid,
                                  const int unresolved_is_fatal) {

  unsigned int j;
  module_data * const midata = &(data->modules[mid]);

  LOG_DEBUG_MONITOR("computing relocation of module id=%u\n", mid);

  for(j = 0; j < midata->nb_functions; ++j) {
    if(FUNCTION_IS_EXTERNAL(midata->functions[j])) {
      int k = data->nb_modules - 1;
      int not_resolved = 1;

      while(not_resolved && (k >= 0)) {
        module_data * const mkdata = &(data->modules[k]);
        unsigned int l = 0;

        if(mid != k) {
          while(not_resolved && (l < mkdata->nb_functions)) {
            if(!FUNCTION_IS_EXTERNAL(mkdata->functions[l]) &&
               (strcmp(midata->functions[j].name, mkdata->functions[l].name) == 0)) {
              midata->relocations[j] = MODULE_FUNCTION(k, l);
              not_resolved = 0;
              LOG_DEBUG_MONITOR("\trelocate %6u -> (%3u,%6u)\n", j, mid, l);
            }
            ++l;
          }
        }
        --k;
      }

      if(not_resolved) {
        LOG_DEBUG_MONITOR("\tno relocation found for id=%u (name=%s)\n", j, midata->functions[j].name);
        if(unresolved_is_fatal) {
          LOG_ERROR_MONITOR("unresolved external function : %s\n", midata->functions[j].name);
          return 1;
        }
      }
    }
  }

  return 0;
}


int monitor_load(const pid_t client_pid,
                 const int fd_client_to_monitor,
                 const int fd_monitor_to_client,
                 const int fd_loading_to_monitor,
                 const int fd_monitor_to_loading,
                 monitor_data * const data) {
  unsigned int i;
  int err, terminated;

  data->client_pid = client_pid;
  data->fd_client_to_monitor = fd_client_to_monitor;
  data->fd_monitor_to_client = fd_monitor_to_client;
  data->fd_loading_to_monitor = fd_loading_to_monitor;
  data->fd_monitor_to_loading = fd_monitor_to_loading;

  data->nb_modules = 0;
  data->modules = NULL;

  terminated = 0;
  do {
    err = monitor_load_module(data);
    if(err > 0) {
      terminated = 1;
      err = 0;
    }
    if(!err && !terminated) {
      err = compute_relocations_of_module(data, data->nb_modules - 1, 0);
      if(!err) {
        const unsigned int nb = data->modules[data->nb_modules - 1].nb_entrypoint_inits;
        i = 0;
        LOG_DEBUG_MONITOR("%u inits are expected\n", nb);
        while((i < nb) && !err) {
          err = monitor_run(*data);
          ++i;
        }
      }
    }
  } while(!terminated && !err);

  i = 0;
  while(!err && (i < data->nb_modules)) {
    err = compute_relocations_of_module(data, i, 1);
    ++i;
  }

  return err;
}


void monitor_data_free(monitor_data * const data) {
  unsigned int i;

  for(i = 0; i < data->nb_modules; ++i) {
    module_data *mdata = &(data->modules[i]);
    unsigned int j;

    for(j = 0; j < mdata->nb_functions; ++j) {
      function_data *fdata = &(mdata->functions[j]);
      unsigned int k;

      if(mdata->call_graph[j]) {
        free(mdata->call_graph[j]);
        mdata->call_graph[j] = NULL;
      }

      for(k = 0; k < fdata->nb_blocks; ++k) {
        if(fdata->control_flow_graph[k]) {
          free(fdata->control_flow_graph[k]);
          fdata->control_flow_graph[k] = NULL;
        }
      }

      free(fdata->name);
      free(fdata->control_flow_graph);
      free(fdata->immediate_post_dominator);
      fdata->name = NULL;
      fdata->control_flow_graph = NULL;
      fdata->immediate_post_dominator = NULL;
      fdata->nb_blocks = 0;
    }

    if(mdata->nb_functions) {
      free(mdata->functions);
      free(mdata->call_graph);
      free(mdata->relocations);
      mdata->functions = NULL;
      mdata->call_graph = NULL;
      mdata->relocations = NULL;
      mdata->nb_functions = 0;
    }
  }

  if(data->nb_modules) {
    free(data->modules);
    data->modules = NULL;
    data->nb_modules = 0;
  }
}
