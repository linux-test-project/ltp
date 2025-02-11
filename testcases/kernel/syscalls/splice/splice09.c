// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test for splicing to /dev/zero and /dev/null these two devices discard all
 * data written to them.
 *
 * The support for splicing to /dev/zero was added in:
 * 1b057bd800c3 ("drivers/char/mem: implement splice() for /dev/zero, /dev/full")
 */

#define _GNU_SOURCE
#include "tst_test.h"

static const char *const test_devices[] = {
	"/dev/null",
	"/dev/zero",
};

static void verify_splice(unsigned int n)
{
	char buf[1024];
	char dev_fd;
	int pipefd[2];

	memset(buf, 0xff, sizeof(buf));

	tst_res(TINFO, "Testing %s", test_devices[n]);

	dev_fd = SAFE_OPEN(test_devices[n], O_WRONLY);

	SAFE_PIPE(pipefd);
	SAFE_WRITE(1, pipefd[1], buf, sizeof(buf));

	TST_EXP_POSITIVE(splice(pipefd[0], NULL, dev_fd, NULL, sizeof(buf), 0));

	if (TST_PASS && TST_RET != sizeof(buf))
		tst_res(TFAIL, "Wrote only part of the pipe buffer");

	SAFE_CLOSE(pipefd[0]);
	SAFE_CLOSE(pipefd[1]);
	SAFE_CLOSE(dev_fd);
}

static struct tst_test test = {
	.test = verify_splice,
	.tcnt = ARRAY_SIZE(test_devices),
	.min_kver = "6.7",
};
