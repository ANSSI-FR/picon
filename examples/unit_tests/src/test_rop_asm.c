#include <stdio.h>
#include <string.h>

void fun_should_not_be_called() {
  printf("fun_should_not_be_called\n");
}

void fun() {
  printf("fun called\n");

  __asm__ ("mov %%rbp,%%rsp; pop %%rbp; push %0; retq;" : : "i"(fun_should_not_be_called));
}

int main() {

  fun();

  return 0;
}
