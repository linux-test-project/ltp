// kills itself using poison
#define _GNU_SOURCE 1
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

#define MADV_POISON 100

int main(void)
{
	int PS = getpagesize();
	char *ptr = mmap(NULL, PS, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, 0,0);;
	printf("ptr = %p\n", ptr);
	madvise(ptr, PS, MADV_POISON);
	printf("faulting\n");
	*ptr = 1;
	printf("waiting\n");
	sleep(100);
	return 0;
}
