/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall add an extra reference to the file
 * associated with the file descriptor fildes which is not removed
 * by a subsequent close() on that file descriptor.
 * This reference shall be removed when there are no more
 * mappings to the file.
 *
 * Test Steps:
 * 1. Create a file, while it is open call unlink().
 * 2. mmap the file to memory, then call close(). If mmap() added
 *    extra reference to the file, the file content should be accesible
 * 3. Acces and msync the mapped file
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
	ssize_t size = 1024;
	int fd, i;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_12_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	unlink(tmpfname);

	if (ftruncate(fd, size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		return PTS_FAIL;
	}

	/* Close the file, now it's referenced only by the mapping */
	close(fd);

	/* Fill the buffer */
	for (i = 0; i < size; i++)
		((char *)pa)[i] = (13 * i) % 21;

	/* Force the data to be written to disk */
	msync(pa, size, MS_SYNC);

	/* Check if the buffer still contains data */
	for (i = 0; i < size; i++) {
		if (((char *)pa)[i] != (13 * i) % 21) {
			printf("FAILED: Mapped buffer was not preserved\n");
			return PTS_FAIL;
		}
	}

	/*
	 * Unmap the buffer, now data should be freed
	 * Unfortunaltely we have no definitive way to check
	 */
	munmap(pa, size);

	printf("Test PASSED\n");
	return PTS_PASS;
}
