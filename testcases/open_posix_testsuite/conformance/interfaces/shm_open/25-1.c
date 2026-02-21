/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the object is truncated to zero length when O_TRUNC is set, the
 * shared memory object exists and it is successfully opened O_RDWR.
 *
 * Steps:
 *  1. Create a shared memory object and set it to a non zero size.
 *  2. Call shm_open with O_TRUNC and O_RDWR set.
 *  3. Check that the shared memory object is zero length using fstat.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_25-1"
#define SHM_SZ 16

int main(void)
{
	int fd;
	struct stat stat_buf;

	/* Create the shared memory object */
	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, SHM_SZ) == -1) {
		perror("An error occurs when calling ftruncate()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (fstat(fd, &stat_buf) != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	shm_unlink(SHM_NAME);

	if (stat_buf.st_size == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("The shared memory object is not zero length.\n");
	return PTS_FAIL;
}
