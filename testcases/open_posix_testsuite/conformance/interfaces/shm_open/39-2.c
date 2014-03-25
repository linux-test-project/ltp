/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = ENAMETOOLONG if the length
 * of the name argument exceeds {PATH_MAX} (including the terminating null).
 *
 * The used name follow the scheme:
 * aaaaaa/aaaaaa/aaaaaa/aaa ...
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

/* Ensure that each component length is short enough */
#define COMPONENT_SIZE _POSIX_NAME_MAX

int main(void)
{
	int fd, i, path_max;
	char *shm_name;

	path_max = pathconf("/", _PC_PATH_MAX);
	if (path_max == -1) {
		perror("An error occurs when calling pathconf()");
		return PTS_UNRESOLVED;
	}
	shm_name = malloc(path_max + 1);

	for (i = 0; i < path_max; i++)
		shm_name[i] = (i + 1) % COMPONENT_SIZE ? 'a' : '/';
	shm_name[path_max] = 0;

	fd = shm_open(shm_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1 && errno == ENAMETOOLONG) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (fd != -1) {
		printf("FAILED: shm_open() succeeded.\n");
		shm_unlink(shm_name);
		return PTS_FAIL;
	}

	if (sysconf(_SC_VERSION) >= 200800L) {
		printf("UNTESTED: shm_open() did not fail with ENAMETOLONG\n");
		return PTS_UNTESTED;
	}

	perror("FAILED: shm_open");
	return PTS_FAIL;
}
