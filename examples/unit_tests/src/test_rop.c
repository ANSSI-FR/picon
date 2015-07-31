#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void fun_should_not_be_called() {
  printf("fun_should_not_be_called\n");
  exit(1);
}

void fun() {
  void (*fun)(void) = fun_should_not_be_called;

  printf("fun called\n");

  *(&fun + 2) = fun_should_not_be_called;
}

int main() {

  fun();

  return 0;
}
