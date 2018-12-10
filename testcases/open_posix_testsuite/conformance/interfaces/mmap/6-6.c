/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The file descriptor fildes shall have been opened with read permission,
 * regardless of the protection options specified. If PROT_WRITE is
 * specified, the application shall ensure that it has opened the file
 * descriptor fildes with write permission unless MAP_PRIVATE
 * is specified in the flags parameter as described below.
 *
 * Test Steps:
 * 1  Open a file with write only permition.
 * 2. Mmap the file to a memory region setting prot as PROT_READ.
 * 3. Get EACCES error when mmap().
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

	/* Create the tmp file */
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_6_6_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	if (ftruncate(fd, size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	close(fd);

	/* Open write only */
	fd = open(tmpfname, O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	/* prot can be set as whatever value */
	pa = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (pa == MAP_FAILED && errno == EACCES) {
		printf("EACCES on attempt to map writeonly file as "
		       "PROT_READ\n" "Test PASSED\n");
		return PTS_PASS;
	}

	printf("Mapping writeonly file with PROT_READ have not "
	       "returned EACCES\n" "Test FAILED\n");
	return PTS_FAIL;
}
