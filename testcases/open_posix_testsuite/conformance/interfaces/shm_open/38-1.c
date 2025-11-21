/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = EMFILE if too many file
 * descriptors are currently in use by this process.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_38-1"

int main(void)
{
	int fd = 0, count = 0;

	while (fd != -1) {
		fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		count++;
	}
	if (errno == EMFILE) {
		shm_unlink(SHM_NAME);
		printf("Test PASSED: %i files open.\n", count);
		return PTS_PASS;
	}
	perror("shm_open");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;
}
