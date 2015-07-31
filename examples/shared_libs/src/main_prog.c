#include <stdio.h>

extern int mylib1_fct1(void);

int main(void)
{
	puts("Hello, world!");

	mylib1_fct1();

	return 0;
}
