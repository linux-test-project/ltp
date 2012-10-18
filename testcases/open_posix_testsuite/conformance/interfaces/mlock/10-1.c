/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that mlock() set errno = EINVAL when addr is not a multiple of
 * {PAGESIZE} if the implementation requires that addr be a multiple of
 * {PAGESIZE}.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

int main() {
        int result;
        long page_size;
        void *ptr, *notpage_ptr;

        page_size = sysconf(_SC_PAGESIZE);
        if (errno) {
                perror("An error occurs when calling sysconf()");
                return PTS_UNRESOLVED;
        }

	ptr = malloc(page_size);
	if (ptr == NULL) {
                printf("Can not allocate memory.\n");
                return PTS_UNRESOLVED;
        }

	notpage_ptr = ((long)ptr % page_size) ? ptr : ptr+1;

	result = mlock(notpage_ptr, page_size - 1);

	if (result == 0) {
		printf("mlock() does not require that addr be a multiple of {PAGESIZE}.\nTest PASSED\n");
		return PTS_PASS;
	} else if (result == -1 && errno == EINVAL) {
		printf("mlock() requires that addr be a multiple of {PAGESIZE}.\nTest PASSED\n");
		return PTS_PASS;
	} else if (errno == EPERM) {
		printf("You don't have permission to lock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	} else if (result != -1) {
		printf("mlock() returns a value of %i instead of 0 or 1.\n",
		       result);
		perror("mlock");
		return PTS_FAIL;
	}
	perror("Unexpected error");
	return PTS_UNRESOLVED;
}
