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
 * 1  Open a file with read only permition.
 * 2. Mmap the file to a memory region setting prot as PROT_WRITE.
 * 3. Setting flag as MAP_PRIVATE.
 * 4. The mmap() should be sucessful.
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
#include "tempfile.h"

int main(void)
{
	char tmpfname[PATH_MAX];
	void *pa;
	size_t size = 1024;
	int fd;

	/* Create the file */
	PTS_GET_TMP_FILENAME(tmpfname, "pts_mmap_6_5");
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

	/* Open it readonly */
	fd = open(tmpfname, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at 2nd open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	pa = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	munmap(pa, size);
	close(fd);

	printf("Successfully mapped readonly file with "
	       "PROT_WRITE, MAP_PRIVATE\n" "Test PASSED\n");
	return PTS_PASS;
}
