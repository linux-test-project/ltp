/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_unlink() function sets errno = ENAMETOOLONG if the length
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
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

/* Ensure that each component length is short enough */
#define COMPONENT_SIZE _POSIX_NAME_MAX

int main(void)
{
	int result, i, path_max;
	char *shm_name;

	path_max = pathconf("/", _PC_PATH_MAX);
	if (path_max == -1) {
		perror("pathconf() failed");
		return PTS_UNRESOLVED;
	}
	shm_name = malloc(path_max + 1);

	if (!shm_name) {
		perror("malloc() failed");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < path_max; i++)
		shm_name[i] = (i + 1) % COMPONENT_SIZE ? 'a' : '/';
	shm_name[path_max] = 0;

	result = shm_unlink(shm_name);

	if (result == -1 && errno == ENAMETOOLONG) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("FAILED: shm_unlink() succeeded.\n");
		return PTS_FAIL;
	}

	if (sysconf(_SC_VERSION) >= 200800L) {
		printf("UNTESTED: shm_open() did not fail with ENAMETOLONG\n");
		return PTS_UNTESTED;
	}

	perror("FAILED: shm_unlink");
	return PTS_FAIL;
}
