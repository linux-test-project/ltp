// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 * 03/30/1992 AUTHOR: Richard Logan CO-PILOT: William Roske
 */

/*\
 * Verify that dup(2) syscall fails with errno EBADF when called with
 * invalid value for oldfd argument.
 */

#include "tst_test.h"

static struct tcase {
	int fd;
	int exp_err;
} tcases[] = {
	{-1, EBADF},
	{1500, EBADF},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL2(dup(tc->fd), tc->exp_err, "dup(%d)", tc->fd);

	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
