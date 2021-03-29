/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2020 Invisible Things Lab
 *                    Michał Kowalczyk <mkow@invisiblethingslab.com>
 */

/*
 * DESCRIPTION
 *  Creates a file using `openat` with bits outside of 07777 in `mode` set and
 *  verifies if they were ignored.
 *
 * WARNING
 *  The fact that these bits are ignored is not documented (at the time of
 *  writing). Failure of this test doesn't necessarily mean that a regression
 *  in Linux was introduced, its intention is to catch accidental interface
 *  changes and warn kernel developers if that happens.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "tst_test.h"

#define TEST_FILE "testfile"

static struct tcase {
	char *filename;
	int flags;
	mode_t mode;
} tcases[] = {
	{TEST_FILE, O_RDWR | O_CREAT, 0644 | ~07777},
	{TEST_FILE, 0, ~07777},
};

static void verify_open(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(openat(AT_FDCWD, tc->filename, tc->flags, tc->mode));
	int fd = TST_RET;
	if (fd == -1) {
		tst_res(TFAIL | TTERRNO, "Cannot open the file");
		return;
	}
	tst_res(TPASS, "Unknown mode bits were ignored as expected");
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.test = verify_open,
};
