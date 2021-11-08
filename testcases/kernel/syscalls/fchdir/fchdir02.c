// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 */

/*
 * DESCRIPTION
 *	fchdir02 - try to cd into a bad directory (bad fd).
 */

#include "tst_test.h"

static void verify_fchdir(void)
{
	const int bad_fd = -5;

	TST_EXP_FAIL(fchdir(bad_fd), EBADF);
}

static struct tst_test test = {
	.test_all = verify_fchdir,
};
