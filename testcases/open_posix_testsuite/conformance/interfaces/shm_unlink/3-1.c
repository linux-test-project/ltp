/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the removal of the memory object contents is postponed until all
 * open and map references to the shared memory object have been removed.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

#define BUF_SIZE 8
#define SHM_NAME "/posixtest_3-1"

int main(void)
{
	int fd;
	char *buf;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, BUF_SIZE) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (shm_unlink(SHM_NAME) != 0) {
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}
	/* Now, SHM_NAME is unlinked but there are open references on it */

	buf = mmap(NULL, BUF_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED && errno == EBADF) {
		printf("The shared memory object was removed.\n");
		return PTS_FAIL;
	} else if (buf == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
