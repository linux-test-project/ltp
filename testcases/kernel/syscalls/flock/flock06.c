// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Matthew Wilcox for Hewlett Packard 2003
 * Author: Matthew Wilcox
 *
 * Test Description:
 *  This test verifies that flock locks held on one fd conflict with flock
 *  locks held on a different fd.
 *
 * Test Steps:
 *  The process opens two file descriptors on the same file.  It acquires
 *  an exclusive flock on the first descriptor, checks that attempting to
 *  acquire an flock on the second descriptor fails.  Then it removes the
 *  first descriptor's lock and attempts to acquire an exclusive lock on
 *  the second descriptor.
 */

#include <errno.h>
#include <sys/file.h>

#include "tst_test.h"

static void verify_flock(void)
{
	int fd1, fd2;

	fd1 = SAFE_OPEN("testfile", O_RDWR);
	TEST(flock(fd1, LOCK_EX | LOCK_NB));
	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "First attempt to flock() failed");
	else
		tst_res(TPASS, "First attempt to flock() passed");

	fd2 = SAFE_OPEN("testfile", O_RDWR);
	TEST(flock(fd2, LOCK_EX | LOCK_NB));
	if (TST_RET == -1)
		tst_res(TPASS | TTERRNO, "Second attempt to flock() denied");
	else
		tst_res(TFAIL, "Second attempt to flock() succeeded!");

	TEST(flock(fd1, LOCK_UN));
	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "Failed to unlock fd1");
	else
		tst_res(TPASS, "Unlocked fd1");

	TEST(flock(fd2, LOCK_EX | LOCK_NB));
	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "Third attempt to flock() denied!");
	else
		tst_res(TPASS, "Third attempt to flock() succeeded");

	SAFE_CLOSE(fd1);
	SAFE_CLOSE(fd2);
}

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0666);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_flock,
	.needs_tmpdir = 1,
	.setup = setup,
};
