/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that mlockall return a value of zero upon successful completion.
 *
 */
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result;

	result = mlockall(MCL_CURRENT);
	if (result == 0 && errno == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (errno == 0) {
		printf("mlockall did not return a value of zero\n");
		return PTS_FAIL;
	} else if (errno == EPERM) {
		printf
		    ("You don't have permission to lock your address space.\nTry to rerun this test as root.\n");
		return PTS_UNRESOLVED;
	}

	perror("Unexpected error");
	return PTS_UNRESOLVED;
}
