/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that shm_open() return a file descriptor for the shared memory object
 * that is the lowest numbered file descriptor not currently open for that
 * process
 *
 * Steps:
 *  1. Open a temporary file.
 *  2. Open a shared memory object.
 *  3. Check that the file descriptor number of the shared memory object
 *     follows the one of the temporary file.
 * It assume that no file descriptor are closed before getting the two file
 * descriptors.
 */

/* mkstemp is an XOPEN extension. */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_8-1"

int main(void)
{
	int fd1, fd2;
	char path[25] = "/tmp/posixtestXXXXXX";

	fd1 = mkstemp(path);
	if (fd1 == -1) {
		perror("An error occurs when calling mkstemp()");
		return PTS_UNRESOLVED;
	}

	fd2 = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd2 == -1) {
		perror("An error occurs when calling shm_open()");
		unlink(path);
		return PTS_UNRESOLVED;
	}

	unlink(path);
	shm_unlink(SHM_NAME);

	if (fd2 == (fd1 + 1)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	printf("Test FAILED\n");
	return PTS_FAIL;
}
