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
 * 1. mmap() a file, setting MAP_SHARED.
 * 2. Modify the mapped memory.
 * 3. Call msync() to synchronize the modification.
 * 4. munmap() this mapping.
 * 5. mmap() the same file again into memory.
 * 6. Check whether the modification has taken effect.
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
	ssize_t size = 1024;
	char data[size];
	void *pa;
	int fd;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_7_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	memset(data, 'a', size);
	if (write(fd, data, size) != size) {
		printf("Error at write(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	*(char *)pa = 'b';

	/* Flush changes back to the file */
	if (msync(pa, size, MS_SYNC) != 0) {
		printf("Error at msync(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	munmap(pa, size);

	/* Mmap again */
	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at 2nd mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	if (*(char *)pa == 'b') {
		printf("Memory write with MAP_SHARED has changed "
		       "the underlying file\n" "Test PASSED\n");

		close(fd);
		munmap(pa, size);

		return PTS_PASS;
	}

	close(fd);
	munmap(pa, size);

	printf("Memory write with MAP_SHARED has not changed "
	       "the underlying file\n" "Test FAILED\n");
	return PTS_FAIL;
}
