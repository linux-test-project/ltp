/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that  the mode is unchanged when O_TRUNC is set, the shared memory
 * object exists and it is successfully opened O_RDWR.
 *
 * Steps:
 *  1. Create a shared memory object.
 *  2. Call shm_open with O_TRUNC and O_RDWR set and with differents permission
 *     mode.
 *  3. Check that the mode is unchanged.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_26-1"
#define CREATION_MODE S_IRUSR|S_IWUSR
#define OPEN_MODE     S_IRGRP

int main(void)
{
	int fd;
	struct stat stat_buf;
	mode_t old_mode;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, CREATION_MODE);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (fstat(fd, &stat_buf) != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}
	old_mode = stat_buf.st_mode;

	fd = shm_open(SHM_NAME, O_RDWR | O_TRUNC, OPEN_MODE);
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

	if (stat_buf.st_mode == old_mode) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("The mode has changed.\n");
	return PTS_FAIL;
}
