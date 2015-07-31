#include <stdio.h>
#include <stdlib.h>


void fun1(const int x) {
  printf("fun1 : %u\n", x);
}

void fun2(const int x) {
  printf("fun2 : %u\n", x);
}

int main() {
  void (*fun)(int);

  if(random()%2) {
    fun = fun1;
  } else {
    fun = fun2;
  }

  fun(10);

  return 0;
}
