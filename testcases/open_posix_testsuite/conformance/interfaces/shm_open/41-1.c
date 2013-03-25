/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = ENOENT if O_CREAT is not set
 * and the named shared memory object does not exist.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_41-1"

int main(void)
{
	int fd, result;

	result = shm_unlink(SHM_NAME);
	if (result != 0 && errno != ENOENT) {
		/* The shared memory object exist and shm_unlink can not
		   remove it. */
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDONLY, S_IRUSR);

	if (fd == -1 && errno == ENOENT) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (fd != -1) {
		printf("shm_open() success.\n");
		return PTS_FAIL;
	}

	perror("Unexpected error");
	return PTS_FAIL;
}
