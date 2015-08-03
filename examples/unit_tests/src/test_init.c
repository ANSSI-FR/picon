#include <stdio.h>

__attribute__((constructor))
void xinit() {
  printf("constructor\n");
}

int main() {
  printf("main\n");
  return 0;
}
