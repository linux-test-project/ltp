/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Upon successful completion, fsync() shall return 0.
 * Otherwise, .1 shall be returned and errno set
 * to indicate the error. If the fsync() function fails.
 *
 * 1. Create a regular file;
 * 2. Write a few bytes to the file, call fsycn(), should return 0;
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define TNAME "fsync/4-1.c"

int main(void)
{
	char tmpfname[256];
	char *data;
	int total_size = 1024;
	int fd;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_fsync_4_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	/* Make sure the file is removed when it is closed */
	unlink(tmpfname);
	data = malloc(total_size);
	memset(data, 'a', total_size);
	if (write(fd, data, total_size) != total_size) {
		printf(TNAME "Error at write(): %s\n", strerror(errno));
		free(data);
		exit(PTS_UNRESOLVED);
	}
	free(data);

	if (fsync(fd) == -1) {
		printf(TNAME "Error at fsync(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
