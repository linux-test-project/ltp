/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that mlock return a value of zero upon successful completion.
 *
 */
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define BUFSIZE 8

int main(void)
{
	int result;
	void *ptr;

	ptr = malloc(BUFSIZE);
	if (ptr == NULL) {
		printf("Can not allocate memory.\n");
		return PTS_UNRESOLVED;
	}

	result = mlock(ptr, BUFSIZE);
	if (result == 0 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == 0) {
		printf("mlock did not return a value of zero\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("You don't have permission to lock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	}

	perror("Unexpected error");
	return PTS_UNRESOLVED;
}
