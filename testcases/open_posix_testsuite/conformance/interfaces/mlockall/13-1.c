/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlockall() function set errno = EINVAL if the flags argument
 * is zero.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result;

	result = mlockall(0);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("mlockall() return %i instead of -1.\n", result);
		return PTS_FAIL;
	} else {
		perror("Unexpected error");
		return PTS_FAIL;
	}
}
