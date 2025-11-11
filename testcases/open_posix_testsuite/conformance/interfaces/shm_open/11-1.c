/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the FD_CLOEXEC file descriptor flag associated with the new file
 * descriptor is set
 *
 * The test use fstat to check the flag.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_11-1"

int main(void)
{
	int fd, flags;

	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	flags = fcntl(fd, F_GETFD);
	if (flags == -1) {
		perror("An error occurs when calling fcntl()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	shm_unlink(SHM_NAME);

	if (flags & FD_CLOEXEC) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	printf("shm_open() does not set the FD_CLOEXEC flags.\n");
	return PTS_FAIL;
}
