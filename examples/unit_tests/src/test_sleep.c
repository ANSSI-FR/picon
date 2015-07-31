#include <unistd.h>
#include <stdio.h>

#define SLEEP_DURATION_SECONDS 2


int main() {

  printf("entering sleep for %u seconds\n", SLEEP_DURATION_SECONDS);

  sleep(SLEEP_DURATION_SECONDS);

  return 0;
}
