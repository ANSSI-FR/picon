#include <unistd.h>
#include <stdio.h>


int main() {
  pid_t child_pid;

  switch((child_pid = fork())) {
  case -1:
    fprintf(stderr, "failed to fork");
    return 1;

  case 0:
    /* child */
    printf("child\n");
    break;

  default:
    /* parent */
    printf("parent\n");
    break;
  }

  return 0;
}
