/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlockall() function set errno = EPERM if the calling process
 * does not have the appropriate privilege to perform the requested operation
 *
 * It is a may assertion.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"


int main() {
	int result;

        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }

	result = mlockall(MCL_CURRENT);

	if(result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if(result == 0) {
		printf("You have the right to call mlockall\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}
}
