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
 * of a pathname component is longer than {NAME_MAX} (not including the
 * terminating null).
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

int main(void)
{
	int fd, i;
	long name_max;
	char *shm_name;

	name_max = pathconf("/", _PC_NAME_MAX);
	if (name_max == -1) {
		perror("An error occurs when calling pathconf()");
		return PTS_UNRESOLVED;
	}
	shm_name = malloc(name_max + 3);

	shm_name[0] = '/';
	for (i = 1; i < name_max + 2; i++)
		shm_name[i] = 'a';
	shm_name[name_max + 2] = 0;

	fd = shm_open(shm_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1 && errno == ENAMETOOLONG) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (fd != -1) {
		printf("FAILED: shm_open() succeeded\n");
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
