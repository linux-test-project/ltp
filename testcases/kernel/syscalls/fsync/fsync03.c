// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) Wayne Boyer, International Business Machines  Corp., 2001
 *   Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * Test Description:
 *  Testcase to check that fsync(2) sets errno correctly.
 *  1. Call fsync() with an invalid fd, and test for EBADF.
 *  2. Call fsync() on a pipe(fd), and expect EINVAL.
 */

#include <unistd.h>
#include <errno.h>
#include "tst_test.h"

static int pfd[2];
static int bfd = -1;

const struct test_case {
	int *fd;
	int error;
} TC[] = {
	/* EBADF - fd is invalid (-1) */
	{&bfd, EBADF},
	/* EINVAL - fsync() on pipe should not succeed. */
	{pfd, EINVAL}
};

static void test_fsync(unsigned int n)
{
	const struct test_case *tc = TC + n;

	TEST(fsync(*tc->fd));

	if (TST_RET != -1) {
		tst_res(TFAIL, "fsync() returned unexpected value %ld",
			TST_RET);
	} else if (TST_ERR != tc->error) {
		tst_res(TFAIL | TTERRNO, "fsync(): unexpected error");
	} else {
		tst_res(TPASS, "fsync() failed as expected");
	}
}

static void setup(void)
{
	SAFE_PIPE(pfd);
}

static void cleanup(void)
{
	close(pfd[0]);
	close(pfd[1]);
}

static struct tst_test test = {
	.test = test_fsync,
	.tcnt = ARRAY_SIZE(TC),
	.setup = setup,
	.cleanup = cleanup
};
