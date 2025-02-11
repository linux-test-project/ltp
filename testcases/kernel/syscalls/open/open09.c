// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2013 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * This test verifies that a file opened with O_RDONLY can't be writable
 * and it verifies that a file opened with O_WRONLY can't be readable.
 */

#include "tst_test.h"

#define TEMPFILE "testfile"

static void setup(void)
{
	SAFE_CREAT(TEMPFILE, 0600);
}

static void verify_open(unsigned int nr)
{
	char pbuf[BUFSIZ];
	int fd = 0;

	if (nr == 0) {
		fd = SAFE_OPEN(TEMPFILE, O_RDONLY);
		TST_EXP_FAIL(write(fd, pbuf, 1), EBADF);
	} else {
		fd = SAFE_OPEN(TEMPFILE, O_WRONLY);
		TST_EXP_FAIL(read(fd, pbuf, 1), EBADF);
	}
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_open,
	.tcnt = 2,
	.needs_tmpdir = 1,
};
