#include <stdio.h>

extern int mylib3_fct1(void);

int mylib2_fct1(void)
{
	printf("%s\n", __FUNCTION__);
	mylib3_fct1();
	return 0;
}

