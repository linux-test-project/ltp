/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shared memory object is created if the shared memory object
 * does not exists and the O_CREAT flags is set.
 *
 * Steps:
 *  1. Ensure that the shared memory object does not exists using shm_unlink.
 *  2. Call shm_open() with O_CREAT flags set.
 *  3. Check that the shared memory object exists now using fstat.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_15-1"

int main(void)
{
	int fd, result;
	struct stat stat_buf;

	result = shm_unlink(SHM_NAME);
	if (result != 0 && errno != ENOENT) {
		/* The shared memory object exist and shm_unlink can not
		   remove it. */
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	result = fstat(fd, &stat_buf);

	if (result == 0) {
		printf("Test PASSED\n");
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if (result == -1 && errno == EBADF) {
		printf("The shared memory object was no created.\n");
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}

	perror("fstat");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;
}
