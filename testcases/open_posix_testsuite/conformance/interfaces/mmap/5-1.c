/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The prot shall be either PROT_NONE or the bitwise-inclusive OR of
 * one or more of the other flags in the following table, defined in the
 * <sys/mman.h> header.
 * PROT_READ Data can be read.
 * PROT_WRITE Data can be written.
 * PROT_EXEC Data can be executed.
 * PROT_NONE Data cannot be accessed.
 * If an implementation cannot support the combination of access types
 * specified by prot, the call to mmap() shall fail.
 *
 * Test Step:
 * 1. mmap(), setting 'prot' as PROT_NONE;
 * 2. mmap(), setting 'prot' as PROT_READ | PROT_WRITE | PROT_EXEC
 *
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
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

int main(void)
{
	char tmpfname[256];
	void *pa = NULL;
	size_t size = 1024;
	int fd;

#ifndef PROT_READ
	printf("Test Fail: PROT_READ not defined\n");
	return PTS_FAIL;
#endif
#ifndef PROT_WRITE
	printf("Test Fail: PROT_WRITE not defined\n");
	return PTS_FAIL;
#endif
#ifndef PROT_EXEC
	printf("Test Fail: PROT_EXEC not defined\n");
	return PTS_FAIL;
#endif
#ifndef PROT_NONE
	printf("Test Fail: PROT_READ not defined\n");
	return PTS_FAIL;
#endif

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_5_1_%d", getpid());
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

	pa = mmap(NULL, size, PROT_NONE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap, with PROT_NONE %s\n", strerror(errno));
		return PTS_FAIL;
	}
	munmap(pa, size);

	pa = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap, with "
		       "PROT_READ | PROT_WRITE | PROT_EXEC %s\n",
		       strerror(errno));
		return PTS_FAIL;
	}
	munmap(pa, size);

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
