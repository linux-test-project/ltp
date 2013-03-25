/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that shm_open() fails if the shared memory object exist and O_EXCL and
 * O_CREAT are set.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_22-1"

int main(void)
{
	int fd;

	/* Create the shared memory object */
	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1 && errno == EEXIST) {
		printf("Test PASSED\n");
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if (fd != -1) {
		printf("Test FAILED\n");
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}

	perror("shm_open");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;
}
