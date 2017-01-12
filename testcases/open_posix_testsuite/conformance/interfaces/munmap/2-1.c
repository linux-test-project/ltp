/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * If there are no mappings in the specified address range, then munmap() has
 * no effect. To get a valid address range which is safe to call munmap() on
 * we first map some arbitrary memory allowing the OS to select the address
 * then unmap it. We then call munmap() on the same address again to perform
 * the test.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define TNAME "munmap/2-1.c"

int main(void)
{
	int rc, fd, map_size;
	void *map_addr;

	map_size = sysconf(_SC_PAGESIZE);
	fd = open("/dev/zero", O_RDWR);
	if (fd == -1) {
		printf("Failed to open /dev/zero: %s (%d)\n",
		       strerror(errno),
		       errno);
		return PTS_UNRESOLVED;
	}

	map_addr = mmap(NULL, map_size, PROT_NONE, MAP_PRIVATE, fd, 0);
	if (map_addr == MAP_FAILED) {
		printf("Failed to map memory: %s (%d)\n",
		       strerror(errno),
		       errno);
		close(fd);
		return PTS_UNRESOLVED;
	}

	close(fd);

	rc = munmap(map_addr, map_size);
	if (rc != 0) {
		printf("Failed to unmap memory: %s (%d)\n",
		       strerror(errno),
		       errno);
		close(fd);
		return PTS_UNRESOLVED;
	}

	rc = munmap(map_addr, map_size);
	if (rc != 0) {
		printf("Test FAILED " TNAME " Error at munmap(): %s (%d)\n",
		       strerror(errno),
		       errno);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
