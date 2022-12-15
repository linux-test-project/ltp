// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2008
 * Email: bnpoorni@in.ibm.com
 */

/*\
 * [Description]
 *
 * Basic error conditions test for sync_file_range() system call, tests for:
 *
 * - EBADFD Wrong filedescriptor
 * - ESPIPE Unsupported file descriptor
 * - EINVAL Wrong offset
 * - EINVAL Wrong nbytes
 * - EINVAL Wrong flags
 */
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/sync_file_range.h"
#include "check_sync_file_range.h"

#ifndef SYNC_FILE_RANGE_WAIT_BEFORE
#define SYNC_FILE_RANGE_WAIT_BEFORE 1
#define SYNC_FILE_RANGE_WRITE 2
#define SYNC_FILE_RANGE_WAIT_AFTER 4
#endif

#define SYNC_FILE_RANGE_INVALID 8

static char filename[255];
static const char spl_file[] = "/dev/null";
static int fd, sfd;
static int bfd = -1;

struct test_case {
	int *fd;
	off_t offset;
	off_t nbytes;
	unsigned int flags;
	int error;
} tcases[] = {
	{&bfd, 0, 1, SYNC_FILE_RANGE_WRITE, EBADF},
	{&sfd, 0, 1, SYNC_FILE_RANGE_WAIT_AFTER, ESPIPE},
	{&fd, -1, 1, SYNC_FILE_RANGE_WAIT_BEFORE, EINVAL},
	{&fd, 0, -1, SYNC_FILE_RANGE_WRITE, EINVAL},
	{&fd, 0, 1, SYNC_FILE_RANGE_INVALID, EINVAL}
};

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	if (!check_sync_file_range())
		tst_brk(TCONF, "sync_file_range() not supported");

	sprintf(filename, "tmpfile_%d", getpid());

	fd = SAFE_OPEN(filename, O_RDWR | O_CREAT, 0700);
	sfd = SAFE_OPEN(spl_file, O_RDWR | O_CREAT, 0700);
}

static void run_test(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	TST_EXP_FAIL(tst_syscall(__NR_sync_file_range, *(tc->fd),
		tc->offset, tc->nbytes, tc->flags), tc->error,
		"sync_file_range(%i, %li, %li, %i)",
		*(tc->fd), (long)tc->offset, (long)tc->nbytes, tc->flags);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run_test,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
