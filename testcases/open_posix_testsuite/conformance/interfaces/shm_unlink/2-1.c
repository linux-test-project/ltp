/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the name is removed before shm_unlink() returns even if one or
 * more references to the shared memory object exist when the object is
 * unlinked.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_2-1"

int main(void)
{
	int fd;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (shm_unlink(SHM_NAME) != 0) {
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDONLY, 0);

	if (fd == -1 && errno == ENOENT) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (fd == -1) {
		perror("shm_open");
		return PTS_UNRESOLVED;
	}

	printf("The name of shared memory object was not removed.\n");
	shm_unlink(SHM_NAME);
	return PTS_FAIL;

}
