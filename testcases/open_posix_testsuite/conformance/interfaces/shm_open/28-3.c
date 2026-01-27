/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the state of the shared memory object, including all data
 * associated with the shared memory object, persists until all open references
 * are gone even if the shared memory object is unlinked and there
 * is no mapping reference anymore.
 *
 * Steps:
 *  1. Create a shared memory object, map it and write a string in it.
 *  2. Unmap the memory and unlink the shared memory object.
 *  3. Remap the shared memory object.
 *  4. Check that the previously written string is always in the memory.
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
#define SHM_NAME "/posixtest_28-3"

int main(void)
{
	int fd;
	char str[BUF_SIZE] = "qwerty";
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

	buf = mmap(NULL, BUF_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	strcpy(buf, str);

	if (munmap(buf, BUF_SIZE) != 0) {
		perror("An error occurs when calling munmap()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (shm_unlink(SHM_NAME) != 0) {
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}
	/* Now, SHM_NAME is unlinked and there are no more mapping references
	   on it but an open reference remain */

	buf = mmap(NULL, BUF_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED && errno == EBADF) {
		printf("The shared memory object was removed.\n");
		return PTS_FAIL;
	} else if (buf == MAP_FAILED) {
		perror("An error occurs when calling mmap()");
		return PTS_UNRESOLVED;
	}

	if (strcmp(buf, str) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	printf("The content of the shared memory object was removed.\n");
	return PTS_FAIL;
}
