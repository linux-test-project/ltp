/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * MAP_SHARED and MAP_PRIVATE describe the disposition of write references
 * to the memory object. If MAP_SHARED is specified, write references shall
 * change the underlying object. If MAP_PRIVATE is specified, modifications
 * to the mapped data by the calling process shall be visible only to the
 * calling process and shall not change the underlying object.
 * It is unspecified whether modifications to the underlying object done
 * after the MAP_PRIVATE mapping is established are visible through
 * the MAP_PRIVATE mapping.
 *
 * Test Steps:
 *
 * 1. Create a shared memory object.
 * 2. mmap the shared memory object into memory, setting MAP_PRIVATE.
 * 3. Modify the mapped memory.
 * 4. Fork a child process.
 * 5. Child process mmap the same shared memory object into memory.
 * 6. Check whether the change in step 3 is visible to the child.
 *
 */

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

int main(void)
{
	char tmpfname[256];
	void *pa;
	size_t size = 1024;
	int fd;
	pid_t child;
	int exit_stat;

	/* Create shared object */
	snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_7_3_%d", getpid());
	shm_unlink(tmpfname);
	fd = shm_open(tmpfname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	shm_unlink(tmpfname);

	if (ftruncate(fd, size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	/* Write the mapped memory */
	*(char *)pa = 'a';

	child = fork();

	switch (child) {
	case 0:
		/* Mmap again the same shared memory to child's memory */
		pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
			  0);
		if (pa == MAP_FAILED) {
			printf("Error at mmap(): %s\n", strerror(errno));
			return PTS_FAIL;
		}

		if (*(char *)pa == 'a') {
			printf("Set flag as MAP_PRIVATE, write reference will "
			       "change the underlying shared memory object\n");
			return PTS_FAIL;
		}

		printf("Set flag as MAP_PRIVATE, write reference will "
		       "not change the underlying shared memory object\n"
		       "Test PASSED\n");
		return PTS_PASS;
		break;
	case -1:
		printf("Error at fork(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
		break;
	default:
		waitpid(child, &exit_stat, WUNTRACED);

		close(fd);
		munmap(pa, size);

		if (WIFEXITED(exit_stat)) {
			return WEXITSTATUS(exit_stat);
		} else {
			printf("Child has not exit properly\n");
			return PTS_UNRESOLVED;
		}
	}
}
