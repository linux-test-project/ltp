// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 *
 * 03/30/1992 AUTHOR: Richard Logan CO-PILOT: William Roske
 *
 */
/*\
 * [DESCRIPTION]
 * Negative test for dup(2) with bad fds.
 *
 * [ALGORITHM]
 * Call dup(2) with invalid argument and make sure it returns -1 with errno set
 * to EBADF.
\*/

#include "tst_test.h"

static struct tcase {
	int fd;
	int expected_errno;
} tcases[] = {
	{-1, EBADF},
	{1500, EBADF},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(dup(tc->fd));

	if (TST_RET < -1) {
		tst_res(TFAIL | TTERRNO, "Invalid dup() return value %ld",
			TST_RET);
		return;
	}

	if (TST_RET == -1) {
		if (tc->expected_errno == TST_ERR) {
			tst_res(TPASS | TTERRNO, "dup(%d) failed as expected",
				tc->fd);
		} else {
			tst_res(TFAIL | TTERRNO, "dup(%d) failed unexpectedly",
				tc->fd);
		}
		return;
	}

	tst_res(TFAIL, "dup(%d) succeeded unexpectedly", tc->fd);
	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
