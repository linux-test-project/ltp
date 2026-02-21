/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_unlink() function sets errno = ENOENT  if the named shared
 * memory object does not exist.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_11-1"

int main(void)
{
	int result;

	/* Ensure that the name SHM_NAME is removed */
	shm_unlink(SHM_NAME);

	result = shm_unlink(SHM_NAME);

	if (result == -1 && errno == ENOENT) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == -1) {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}

	printf("shm_unlink() success.\n");
	return PTS_FAIL;

}
