// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test for splicing from /dev/zero and /dev/full.
 *
 * The support for splicing from /dev/zero and /dev/full was removed in:
 * c6585011bc1d ("splice: Remove generic_file_splice_read()")
 *
 * And added back in:
 * 1b057bd800c3 ("drivers/char/mem: implement splice() for /dev/zero, /dev/full")
 */

#define _GNU_SOURCE
#include "tst_test.h"

static int fd_zero;
static int fd_full;

static void test_splice(unsigned int bytes, int dev_fd)
{
	int pipefd[2];
	char buf[bytes];
	size_t i;
	int fail = 0;

	memset(buf, 0xff, sizeof(buf));

	SAFE_PIPE(pipefd);

	TST_EXP_POSITIVE(splice(dev_fd, NULL, pipefd[1], NULL, sizeof(buf), 0));

	if (!TST_PASS)
		goto ret;

	if ((size_t)TST_RET != sizeof(buf)) {
		tst_res(TFAIL, "Only part %lu bytes of %u were spliced",
			TST_RET, bytes);
		goto ret;
	}

	SAFE_READ(1, pipefd[0], buf, sizeof(buf));

	for (i = 0; i < sizeof(buf); i++) {
		if (buf[i])
			fail++;
	}

	if (fail)
		tst_res(TFAIL, "%i non-zero bytes spliced from /dev/zero", fail);
	else
		tst_res(TPASS, "All bytes spliced from /dev/zero are zeroed");

ret:
	SAFE_CLOSE(pipefd[0]);
	SAFE_CLOSE(pipefd[1]);
}

static void verify_splice(unsigned int n)
{
	unsigned int bytes = 1009 * n;

	tst_res(TINFO, "Splicing %u bytes from /dev/zero", bytes);
	test_splice(bytes, fd_zero);
	tst_res(TINFO, "Splicing %u bytes from /dev/full", bytes);
	test_splice(bytes, fd_full);
}

static void setup(void)
{
	fd_zero = SAFE_OPEN("/dev/zero", O_RDONLY);
	fd_full = SAFE_OPEN("/dev/full", O_RDONLY);
}

static void cleanup(void)
{
	if (fd_zero > 0)
		SAFE_CLOSE(fd_zero);

	if (fd_full > 0)
		SAFE_CLOSE(fd_full);
}

static struct tst_test test = {
	.test = verify_splice,
	.tcnt = 9,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "6.7",
};
