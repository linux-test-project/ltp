// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc. All Rights Reserved.
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 * AUTHOR            : William Roske
 * CO-PILOT          : Dave Fenner
 */

/*\
 * Basic test for fcntl(2) using F_GETLK argument.
 *
 * If the lock could be placed, fcntl() does not actually place it, but
 * returns F_UNLCK in the l_type field of lock and leaves the other field
 * of the structure unchanged.
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "tst_test.h"

static int fd = -1, pid;
static struct flock flocks;

static void verify_fcntl(void)
{
	/* F_GETLK will change flock.l_type to F_UNLCK, so need to reset */
	flocks.l_type = F_RDLCK;

	TST_EXP_PASS(fcntl(fd, F_GETLK, &flocks), "fcntl(%d, F_GETLK, &flocks)", fd);
	TST_EXP_EQ_LI(flocks.l_type, F_UNLCK);
	TST_EXP_EQ_LI(flocks.l_whence, SEEK_CUR);
	TST_EXP_EQ_LI(flocks.l_start, 0);
	TST_EXP_EQ_LI(flocks.l_len, 0);
	TST_EXP_EQ_LI(flocks.l_pid, pid);
}

static void setup(void)
{
	pid = getpid();
	fd = SAFE_OPEN("filename", O_RDWR | O_CREAT, 0700);

	flocks.l_whence = SEEK_CUR;
	flocks.l_start = 0;
	flocks.l_len = 0;
	flocks.l_pid = pid;
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = verify_fcntl,
	.setup = setup,
	.cleanup = cleanup,
};
