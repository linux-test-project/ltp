/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that shm_open open the shared memory object for reading when calling
 * shm_open even if the mode don't set read permission.
 *
 * The test use mmap to check the object is open for reading.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

#define BUF_SIZE 8
#define SHM_NAME "posixtest_20-3"

int main(void)
{
	int fd, result;
	void *ptr;

	result = shm_unlink(SHM_NAME);
	if (result != 0 && errno != ENOENT) {
		/* The shared memory object exist and shm_unlink can not
		   remove it. */
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, BUF_SIZE) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	ptr = mmap(NULL, BUF_SIZE, PROT_NONE, MAP_SHARED, fd, 0);
	if (ptr != MAP_FAILED) {
		printf("Test PASSED\n");
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if (errno == EACCES) {
		printf("The shared memory object is not opened for reading.\n");
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}

	perror("mmap");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;
}
