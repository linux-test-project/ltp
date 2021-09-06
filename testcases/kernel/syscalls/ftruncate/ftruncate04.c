// SPDX-License-Identifier: GPL-2.0-or-later
 /*
  * Copyright (c) International Business Machines  Corp., 2002
  * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
  * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
  *
  * Robbie Williamson <robbiew@us.ibm.com>
  * Roy Lee <roylee@andestech.com>
  */
/*
 * Test Description:
 *
 * Tests truncate and mandatory record locking.
 *
 * Parent creates a file, child locks a region and sleeps.
 *
 * Parent checks that ftruncate before the locked region and inside the region
 * fails while ftruncate after the region succeds.
 *
 * Parent wakes up child, child exits, lock is unlocked.
 *
 * Parent checks that ftruncate now works in all cases.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#include "tst_test.h"

#define RECLEN	100
#define MNTPOINT	"mntpoint"
#define TESTFILE	MNTPOINT"/testfile"

static int len = 8 * 1024;
static int recstart, reclen;

static void ftruncate_expect_fail(int fd, off_t offset, const char *msg)
{
	TEST(ftruncate(fd, offset));

	if (TST_RET == 0) {
		tst_res(TFAIL, "ftruncate() %s succeeded unexpectedly", msg);
		return;
	}

	if (TST_ERR != EAGAIN) {
		tst_res(TFAIL | TTERRNO,
			"ftruncate() %s failed unexpectedly, expected EAGAIN",
			msg);
		return;
	}

	tst_res(TPASS, "ftruncate() %s failed with EAGAIN", msg);
}

static void ftruncate_expect_success(int fd, off_t offset, const char *msg)
{
	struct stat sb;

	TEST(ftruncate(fd, offset));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"ftruncate() %s failed unexpectedly", msg);
		return;
	}

	SAFE_FSTAT(fd, &sb);

	if (sb.st_size != offset) {
		tst_res(TFAIL,
			"ftruncate() to %li bytes succeded but fstat() reports size %li",
			(long)offset, (long)sb.st_size);
		return;
	}

	tst_res(TPASS, "ftruncate() %s succeded", msg);
}

static void doparent(void)
{
	int fd;

	TST_CHECKPOINT_WAIT(0);

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_NONBLOCK);

	ftruncate_expect_fail(fd, RECLEN, "offset before lock");
	ftruncate_expect_fail(fd, recstart + RECLEN/2, "offset in lock");
	ftruncate_expect_success(fd, recstart + RECLEN, "offset after lock");

	TST_CHECKPOINT_WAKE(0);
	SAFE_WAIT(NULL);

	ftruncate_expect_success(fd, recstart + RECLEN/2, "offset in lock");
	ftruncate_expect_success(fd, recstart, "offset before lock");
	ftruncate_expect_success(fd, recstart + RECLEN, "offset after lock");

	SAFE_CLOSE(fd);
}

void dochild(void)
{
	int fd;
	struct flock flocks;

	fd = SAFE_OPEN(TESTFILE, O_RDWR);

	tst_res(TINFO, "Child locks file");

	flocks.l_type = F_WRLCK;
	flocks.l_whence = SEEK_CUR;
	flocks.l_start = recstart;
	flocks.l_len = reclen;

	SAFE_FCNTL(fd, F_SETLKW, &flocks);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_res(TINFO, "Child unlocks file");

	exit(0);
}

static void verify_ftruncate(void)
{
	int pid;

	if (tst_fill_file(TESTFILE, 0, 1024, 8))
		tst_brk(TBROK, "Failed to create test file");

	SAFE_CHMOD(TESTFILE, 02666);

	reclen = RECLEN;
	recstart = RECLEN + rand() % (len - 3 * RECLEN);

	pid = SAFE_FORK();

	if (pid == 0)
		dochild();

	doparent();
}

static void setup(void)
{
	 /*
	  * Kernel returns EPERM when CONFIG_MANDATORY_FILE_LOCKING is not
	  * supported - to avoid false negatives, mount the fs first without
	  * flags and then remount it as MS_MANDLOCK
	  */
	if (mount(NULL, MNTPOINT, NULL, MS_REMOUNT|MS_MANDLOCK, NULL) == -1) {
		if (errno == EPERM) {
			tst_brk(TCONF,
				"Mandatory lock not supported by this system");
		} else {
			tst_brk(TBROK | TTERRNO,
				"Remount with MS_MANDLOCK failed");
		}
	}
}

static struct tst_test test = {
	.needs_kconfigs = (const char *[]) {
		"CONFIG_MANDATORY_FILE_LOCKING=y",
		NULL
	},
	.test_all = verify_ftruncate,
	.setup = setup,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.mount_device = 1,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
};
