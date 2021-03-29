/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2020 Invisible Things Lab
 *                    Michał Kowalczyk <mkow@invisiblethingslab.com>
 */

/*
 * DESCRIPTION
 *  Changes file access permissions using `chmod` with bits outside of 07777 in
 *  `mode` set and verifies if they were ignored.
 *
 * WARNING
 *  The fact that these bits are ignored is not documented (at the time of
 *  writing). Failure of this test doesn't necessarily mean that a regression
 *  in Linux was introduced, its intention is to catch accidental interface
 *  changes and warn kernel developers if that happens.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tst_test.h"

#define OPEN_MODE  0644
#define CHMOD_MODE (0777 | ~07777)
#define TESTFILE   "testfile"

static void test_chmod(void)
{
	struct stat stat_buf;

	TEST(chmod(TESTFILE, CHMOD_MODE));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "chmod(%s, %#o) failed", TESTFILE, CHMOD_MODE);
		return;
	}

	SAFE_STAT(TESTFILE, &stat_buf);

	mode_t expected = S_IFREG | (CHMOD_MODE & 07777);
	if (stat_buf.st_mode != expected) {
		tst_res(TFAIL, "%s: Incorrect mode 0%04o, expected 0%04o",
				TESTFILE, stat_buf.st_mode, expected);
		return;
	}
	tst_res(TPASS, "Unknown mode bits were ignored as expected");
}

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN(TESTFILE, O_RDWR | O_CREAT, OPEN_MODE);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = test_chmod,
};
