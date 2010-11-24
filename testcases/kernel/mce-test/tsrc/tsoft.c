/* Simplest soft offline testcase */
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

#define err(x) perror(x), exit(1)

#define MADV_SOFT_OFFLINE 101          /* soft offline page for testing */

int PS;

int main(void)
{
	PS = getpagesize();
	char *map = mmap(NULL, PS,  PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);

	if (map == (char *)-1L)
		err("mmap");

	*map = 1;

	if (madvise(map, PS, MADV_SOFT_OFFLINE) < 0) 
		perror("madvise SOFT_OFFLINE");
	
	*map = 2;

	return 0;
}
