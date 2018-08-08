/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	The function will cause the space pointed to by ptr to be deallocated.
 *
 * method:
 *	-Set rlimit equals to 1024 * 1024 * 2.
 *	-Allocate memory to pointer_1 of size 1024 * 1024 *1.
 *	-Alloacte memory for pointer_2 of size 1024 *1024 *1.
 *	But it will fail due to insufficient memory.
 *	-Try allocating memory to pointer_2 after freeing memory
 *	allocated to pointer_1.
 *	-Check whether memory is allocated by malloc().
 *
*/

#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "posixtest.h"

#define TNAME "free/1-1.c"

int main(void)
{
	int ret = 0;
	struct rlimit rlim;
	char *ptr1;
	char *ptr2;

	rlim.rlim_cur = 1024 * 1024 * 2;

	errno = 0;
	ret = setrlimit(RLIMIT_DATA, &rlim);
	if (ret == -1) {
		printf(TNAME " Error at setrlimit(), errno: %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ptr1 = malloc(1024 * 1024 * 1);
	/* Malloc needs some memeory for its own data structure so more than
	1024 * 1024 * 1 is used here */
	if (ptr1 == NULL) {
		printf(TNAME " Malloc failed(), errno: %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = 0;
	ptr2 = malloc(1024 * 1024 * 1);
	if (ptr2 != NULL) {
		printf(TNAME " Test Failed, malloc() is able to allocate memory before free(),"
				" errno: %d.\n", errno);
		exit(PTS_FAIL);
	}
	free(ptr1);
	errno = 0;
	ptr2 = malloc(1024 * 1024 * 1);
	if (ptr2 == NULL) {
		printf(TNAME " Test Failed, malloc() failed to allocate memory even after"
				" freeing previously allocated memory, errno: %d\n.", errno);
		exit(PTS_FAIL);
	} else {
		printf(TNAME " Test Passed, malloc() allocated memory after freeing"
				" previously allocated memory.\n");
		free(ptr2);
		exit(PTS_PASS);
	}
}
