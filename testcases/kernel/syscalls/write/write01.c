// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Fujitsu Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tst_test.h"

static int fd;

static void verify_write(void)
{
	int i, badcount = 0;
	char buf[BUFSIZ];

	memset(buf, 'w', BUFSIZ);

	SAFE_LSEEK(fd, 0, SEEK_SET);

	for (i = BUFSIZ; i > 0; i--) {
		TEST(write(fd, buf, i));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "write failed");
			return;
		}

		if (TST_RET != i) {
			badcount++;
			tst_res(TINFO, "write() returned %ld, expected %d",
				TST_RET, i);
		}
	}

	if (badcount != 0)
		tst_res(TFAIL, "write() failed to return proper count");
	else
		tst_res(TPASS, "write() passed");
}

static void setup(void)
{
	fd = SAFE_OPEN("test_file", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
	.needs_tmpdir = 1,
};
