#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#include <picon/shared.h>

#define UNUSED __attribute__((unused))
#define HIDDEN __attribute__((visibility("hidden")))
#define INTERNAL __attribute__((visibility("internal")))

static int _fd_client_to_monitor = -1;
static int _fd_monitor_to_client = -1;

static INTERNAL client_signal _cs = { .module = (uint16_t)-1 };

__attribute__((always_inline)) static inline void send_monitor(const client_signal cs) {
  monitor_answer ma;
  int err = 0;

  WRITE_CLIENT_SIGNAL(_fd_client_to_monitor,cs,err);
  if (err) abort();

  READ_MONITOR_ANSWER(_fd_monitor_to_client,ma,err);
  if (err) abort();
  if (ma.event != CFI_OK) abort();
}

#if 0
void __CFI_INTERNAL_FORK_SON(uint32_t nFct, uint32_t nModule) {
  const char *s = getenv("CFI_FD_1");
  const char *sr = getenv("CFI_FD_R_1");
  char *ptr = NULL;
  int ofd = fd;
  int ofdr = fd_r;

  if (s == NULL) {
    assert(0 && "CFI_FD not set in ENV");
  } else if (sr == NULL) {
    assert(0 && "CFI_FD_R not set in ENV");
  }

  fd = strtol(s, &ptr, 10);
  assert(ptr != NULL  && "CFI_FD bad format");
  fd_r = strtol(sr, &ptr, 10);
  assert(ptr != NULL  && "CFI_FD_R bad format");

  unsetenv("CFI_FD");
  unsetenv("CFI_FD_R");

  bzero(&p, sizeof(p));

  p.id = FORK_SON;
  p.param1 = fd_r;
  p.param2 = fd;

  owrite(ofd, (packet *) &p , sizeof(p));
}
#endif

void HIDDEN __CFI_SET_FDS(int fd_client_to_monitor, int fd_monitor_to_client)
{
  _fd_client_to_monitor = fd_client_to_monitor;
  _fd_monitor_to_client = fd_monitor_to_client;
}

void HIDDEN __CFI_SET_MODULE_ID(uint16_t m_id)
{
  _cs.module = m_id;
}

void HIDDEN __CFI_INTERNAL_ENTER(uint32_t nFct, void * UNUSED retaddr) {
  _cs.event = CFI_ENTER;
  _cs.function = nFct;
  _cs.block = 0;

  send_monitor(_cs);
}

void HIDDEN __CFI_INTERNAL_EXIT(uint32_t nFct, void * UNUSED retaddr) {
  _cs.event = CFI_EXIT;
  _cs.function = nFct;
  _cs.block = 0;

  send_monitor(_cs);
}

void HIDDEN __CFI_INTERNAL_CALL(uint32_t nFct) {
  _cs.event = CFI_CALL;
  _cs.function = nFct;
  _cs.block = 0;

  send_monitor(_cs);
}

void HIDDEN __CFI_INTERNAL_RETURNED(uint32_t nFct) {
  _cs.event = CFI_RETURNED;
  _cs.function = nFct;
  _cs.block = 0;

  send_monitor(_cs);
}

void HIDDEN __CFI_INTERNAL_BB_BEFORE_BR(uint32_t f_id, uint32_t idBB) {
  _cs.event = CFI_BEFORE_JUMP;
  _cs.function = f_id;
  _cs.block = idBB;

  send_monitor(_cs);
}

void HIDDEN __CFI_INTERNAL_BB_AFTER_BR(uint32_t f_id, uint32_t idBB) {
  _cs.event = CFI_AFTER_JUMP;
  _cs.function = f_id;
  _cs.block = idBB;

  send_monitor(_cs);
}
