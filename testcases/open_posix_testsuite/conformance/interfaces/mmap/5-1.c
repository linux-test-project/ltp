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
 * Test Steps:
 * 1. call mmap() for all combinations permitted by POSIX
 * 2. each should either succed or fail with ENOTSUP
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

struct testcase {
	int prot;
	int flags;
};

struct testcase testcases[] = {
	{.flags = MAP_SHARED,.prot = PROT_NONE},
	{.flags = MAP_SHARED,.prot = PROT_READ},
	{.flags = MAP_SHARED,.prot = PROT_WRITE},
	{.flags = MAP_SHARED,.prot = PROT_EXEC},
	{.flags = MAP_SHARED,.prot = PROT_READ | PROT_WRITE},
	{.flags = MAP_SHARED,.prot = PROT_READ | PROT_EXEC},
	{.flags = MAP_SHARED,.prot = PROT_EXEC | PROT_WRITE},
	{.flags = MAP_SHARED,.prot = PROT_READ | PROT_WRITE | PROT_EXEC},

	{.flags = MAP_PRIVATE,.prot = PROT_NONE},
	{.flags = MAP_PRIVATE,.prot = PROT_READ},
	{.flags = MAP_PRIVATE,.prot = PROT_WRITE},
	{.flags = MAP_PRIVATE,.prot = PROT_EXEC},
	{.flags = MAP_PRIVATE,.prot = PROT_READ | PROT_WRITE},
	{.flags = MAP_PRIVATE,.prot = PROT_READ | PROT_EXEC},
	{.flags = MAP_PRIVATE,.prot = PROT_EXEC | PROT_WRITE},
	{.flags = MAP_PRIVATE,.prot = PROT_READ | PROT_WRITE | PROT_EXEC},
};

static void print_error(struct testcase *t, int saved_errno)
{
	printf("Combination of ");

	if (t->prot == PROT_NONE)
		printf("PROT_NONE ");

	if (t->prot & PROT_READ)
		printf("PROT_READ ");

	if (t->prot & PROT_WRITE)
		printf("PROT_WRITE ");

	if (t->prot & PROT_EXEC)
		printf("PROT_EXEC ");

	switch (t->flags) {
	case MAP_SHARED:
		printf("with MAP_SHARED");
		break;
	case MAP_PRIVATE:
		printf("with MAP_PRIVATE");
		break;
	}

	printf(" has failed: %s\n", strerror(saved_errno));
}

int main(void)
{
	char tmpfname[256];
	void *pa;
	size_t size = 1024;
	int fd, fail = 0;
	unsigned int i;

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

	for (i = 0; i < sizeof(testcases) / sizeof(*testcases); i++) {

		pa = mmap(NULL, size, testcases[i].prot, testcases[i].flags, fd,
			  0);

		if (pa == MAP_FAILED) {
			if (errno != ENOTSUP) {
				print_error(&testcases[i], errno);
				fail++;
			}
		} else {
			munmap(pa, size);
		}
	}

	close(fd);

	if (fail)
		return PTS_FAIL;

	printf("Test PASSED\n");
	return PTS_PASS;
}
