/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlockall() function set errno = ENOMEM if locking all of the
 * pages currently mapped into the address space of the process would exceed an
 * implementation-defined limit on the amount of memory that the process may
 * lock
 *
 * It is a may assertion.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

/* The number of page in each allocated block */
#define NPAGE 32

int main() {
	int result;
	size_t block_size;
	void *ptr;

	block_size = NPAGE * sysconf(_SC_PAGESIZE);
	if(errno) {
		perror("An error occurs when calling sysconf()");
		return PTS_UNRESOLVED;
	}


	while( (result = mlockall(MCL_CURRENT)) == 0 ) {
		ptr = malloc(block_size);
		if(ptr == NULL) {
			printf("Can not allocate memory.\n");
			return PTS_UNRESOLVED;
		}
	}

	if(result == -1 && errno == ENOMEM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if(result != -1) {
		printf("mlockall returns %i instead of -1.\n", result);
		return PTS_FAIL;
	} else if(errno == EPERM){
		printf("You don't have permission to lock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}
}
