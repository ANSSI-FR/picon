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
#include "monitor_load.h"
#include "monitor_run.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>



static
int mask_sigpipe() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  return sigaction(SIGPIPE, &sa, 0);
}



int main (int argc, char *argv[]) {
  char buffer[128];
  int fd_client_to_monitor[2];
  int fd_monitor_to_client[2];
  int fd_loading_to_monitor[2];
  int fd_monitor_to_loading[2];

  pid_t child_pid;

  unsigned int err = 0;
  monitor_data data;

  if(getenv("LD_PRELOAD")) {
    LOG_ERROR_MONITOR("LD_PRELOAD detected, aborting\n");
    return -1;
  }

  if((getenv(ENV_FD_CLIENT_TO_MONITOR)) ||
     (getenv(ENV_FD_MONITOR_TO_CLIENT)) ||
     (getenv(ENV_FD_LOADING_TO_MONITOR)) ||
     (getenv(ENV_FD_MONITOR_TO_LOADING))) {
    LOG_ERROR_MONITOR("inception of monitored programs is not supported\n");
    return -1;
  }

  if(argc < 2) {
    LOG_ERROR_MONITOR("usage: %s prog [prog_args ...]\n", argv[0]);
    return -2;
  }

  if(getenv(ENV_SM_TRACE)) {
    LOG_DEBUG_MONITOR("%s set, so we are keeping traces\n", ENV_SM_TRACE);
    OPTION_sm_trace = 1;
  }

  if(getenv(ENV_SM_DUMP)) {
    OPTION_sm_dump = 1;
  }

  if(pipe2(fd_client_to_monitor, O_DIRECT)) {
    LOG_ERROR_MONITOR("failed to create client to monitor pipe : %s\n", strerror(errno));
    return -3;
  }

  if(pipe2(fd_monitor_to_client, O_DIRECT)) {
    LOG_ERROR_MONITOR("failed to create monitor to client pipe : %s\n", strerror(errno));
    return -3;
  }

  if(pipe2(fd_loading_to_monitor, O_DIRECT)) {
    LOG_ERROR_MONITOR("failed to create loading to monitor pipe : %s\n", strerror(errno));
    return -3;
  }

  if(pipe2(fd_monitor_to_loading, O_DIRECT)) {
    LOG_ERROR_MONITOR("failed to create monitor to loading pipe : %s\n", strerror(errno));
    return -3;
  }


  LOG_DEBUG_MONITOR("forking\n");

  if((child_pid = fork()) > 0) {
    /* monitor process */
    close(fd_client_to_monitor[1]);
    close(fd_monitor_to_client[0]);
    close(fd_loading_to_monitor[1]);
    close(fd_monitor_to_loading[0]);

    if((err = mask_sigpipe())) {
      LOG_ERROR_MONITOR("cannot mask SIGPIPE\n");
    } else if((err = monitor_load(child_pid,
                                  fd_client_to_monitor[0],
                                  fd_monitor_to_client[1],
                                  fd_loading_to_monitor[0],
                                  fd_monitor_to_loading[1],
                                  &data))) {
      LOG_ERROR_MONITOR("failed to load monitor data\n");
    } else {

      if(OPTION_nb_preload_monitor_ok_answers > 0) {
        const monitor_answer ok_resp = { .event = CFI_OK };
        unsigned int i = 0;
        LOG_DEBUG_MONITOR("preloading %u OK answers\n", OPTION_nb_preload_monitor_ok_answers);
        while((i < OPTION_nb_preload_monitor_ok_answers) && !err) {
          WRITE_MONITOR_ANSWER(data.fd_monitor_to_client, ok_resp, err);
          ++i;
        }
      }

      LOG_DEBUG_MONITOR("main loop\n");
      err = monitor_run(data);
      if(!err) {
        unsigned int i;
        unsigned int nb = 0;
        for(i = 0; i < data.nb_modules; ++i) {
          nb += data.modules[i].nb_entrypoint_finis;
        }
        i = 0;
        LOG_DEBUG_MONITOR("%u finis are expected\n", nb);
        while((i < nb) && !err) {
          err = monitor_run(data);
          ++i;
        }
      }
    }

    LOG_DEBUG_MONITOR("exits with status = %i\n", err);

    if(err) {
      LOG_DEBUG_MONITOR("killing client\n");
      kill(child_pid, SIGKILL);
    }

    monitor_data_free(&data);

    return err;

  } else if(child_pid == 0) {

    /* client process */
    close(fd_client_to_monitor[0]);
    close(fd_monitor_to_client[1]);
    close(fd_loading_to_monitor[0]);
    close(fd_monitor_to_loading[1]);

    snprintf(buffer, sizeof(buffer), "%u", fd_client_to_monitor[1]);
    setenv(ENV_FD_CLIENT_TO_MONITOR, buffer, 1);

    snprintf(buffer, sizeof(buffer), "%u", fd_monitor_to_client[0]);
    setenv(ENV_FD_MONITOR_TO_CLIENT, buffer, 1);

    snprintf(buffer, sizeof(buffer), "%u", fd_loading_to_monitor[1]);
    setenv(ENV_FD_LOADING_TO_MONITOR, buffer, 1);

    snprintf(buffer, sizeof(buffer), "%u", fd_monitor_to_loading[0]);
    setenv(ENV_FD_MONITOR_TO_LOADING, buffer, 1);

    LOG_DEBUG_CLIENT("exec\n");
    ++argv;
    (void)execve(argv[0], argv, environ);

    LOG_ERROR_MONITOR("failed to exec : %s\n", strerror(errno));
    return -4;
  }

  /* error during fork */
  LOG_ERROR_MONITOR("failed to fork : %s\n", strerror(errno));
  return -5;
}
