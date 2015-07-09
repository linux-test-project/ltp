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
 * of a pathname component is longer than {NAME_MAX} (not including the
 * terminating null).
 */

#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

int main(void)
{
	int result, i;
	long name_max;
	char *shm_name;

	name_max = pathconf("/", _PC_NAME_MAX);

	if (name_max == -1) {
		perror("pathconf() failed");
		return PTS_UNRESOLVED;
	}

	shm_name = malloc(name_max + 3);

	if (!shm_name) {
		perror("malloc() failed");
		return PTS_UNRESOLVED;
	}

	shm_name[0] = '/';
	for (i = 1; i < name_max + 2; i++)
		shm_name[i] = 'a';
	shm_name[name_max + 2] = 0;

	result = shm_unlink(shm_name);

	if (result == -1 && errno == ENAMETOOLONG) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result != -1) {
		printf("FAILED: shm_unlink() succeeded\n");
		return PTS_FAIL;
	}

	if (sysconf(_SC_VERSION) >= 200800L) {
		printf("UNTESTED: shm_open() did not fail with ENAMETOLONG\n");
		return PTS_UNTESTED;
	}

	perror("FAILED: shm_unlink");
	return PTS_FAIL;
}
