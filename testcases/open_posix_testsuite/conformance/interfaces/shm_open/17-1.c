/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the goup ID of the shared memory object is set to the effective
 * group ID of the process when the shared memory object does not exists and
 * the O_CREAT flags is set.
 *
 * The test use fstat to check the flag.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "/posixtest_17-1"

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
	if (result != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	shm_unlink(SHM_NAME);

	if (stat_buf.st_gid == getegid()) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	printf
	    ("shm_open() does not set the user ID to the effective user ID of the process.\n");
	return PTS_FAIL;
}
