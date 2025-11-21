/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the file is not open for write access when the applications
 * specify the value of O_RDONLY.
 *
 * The test uses ftruncate to check that the file is not open for write access.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_13-1"
#define BUF_SIZE 8

int main(void)
{
	int fd, result;

	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	result = ftruncate(fd, BUF_SIZE);

	if (result == -1 && errno == EINVAL) {
		printf("Test PASSED\n");
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if (result == 0) {
		printf("The file is open for write acces.\n");
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}

	perror("ftruncate");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;
}
