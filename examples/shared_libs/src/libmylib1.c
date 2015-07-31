#include <stdio.h>

extern int mylib2_fct1(void);

static void __attribute__((constructor)) mylib1_init(void)
{
	printf("Init function %s\n", __FUNCTION__);
}

int mylib1_fct1(void)
{
	printf("%s\n", __FUNCTION__);
	mylib2_fct1();
	return 0;
}

static void __attribute__((destructor)) mylib1_fini(void)
{
	printf("Fini function %s\n", __FUNCTION__);
}

