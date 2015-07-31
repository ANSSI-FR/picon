#include <stdio.h>
#include <stdlib.h>

#include <picon/picon.h>



CFI_IGNORE_BASIC_BLOCKS
void
fill_incr(unsigned int * const tab,
          const unsigned int width,
          const unsigned int height) {

  unsigned int i, j, count;

  count = 0;
  for(i = 0; i < width; ++i) {
    for(j = 0; j < height; ++j) {
      tab[i * height + j] = count++;
    }
  }
}


CFI_IGNORE_BASIC_BLOCKS
void
do_incr(unsigned int * const tab,
        const unsigned int width,
        const unsigned int height) {

  unsigned int i, j;

  for(i = 0; i < width; ++i) {
    for(j = 0; j < height; ++j) {
      ++(tab[i * height + j]);
    }
  }
}




#define SOME_WIDTH 4000
#define SOME_HEIGHT SOME_WIDTH



int main() {

  unsigned int *tab;

  fprintf(stdout, "allocating tab with %u integers\n", SOME_WIDTH * SOME_HEIGHT);
  fflush(stdout);
  if((tab = calloc(SOME_WIDTH * SOME_HEIGHT, sizeof(unsigned int))) == NULL) {
    perror("failed to allocate tab");
    return 1;
  }

  fprintf(stdout, "filling tab\n");
  fflush(stdout);
  fill_incr(tab, SOME_WIDTH, SOME_HEIGHT);

  fprintf(stdout, "incrementing tab\n");
  fflush(stdout);
  do_incr(tab, SOME_WIDTH, SOME_HEIGHT);

  fprintf(stdout, "freeing tab\n");
  fflush(stdout);
  free(tab);

  return 0;
}
