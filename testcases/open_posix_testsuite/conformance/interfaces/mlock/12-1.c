/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlock() function sets errno = EPERM if the calling process
 * does not have the appropriate privilege to perform the requested operation.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define BUFSIZE 8

int main() {
        int result;
        void *ptr;

        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }

	ptr = malloc(BUFSIZE);
	if(ptr == NULL) {
                printf("Can not allocate memory.\n");
                return PTS_UNRESOLVED;
        }

	result = mlock(ptr, BUFSIZE);

	if(result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if(result == 0) {
		printf("You have the right to call mlock\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}

}
