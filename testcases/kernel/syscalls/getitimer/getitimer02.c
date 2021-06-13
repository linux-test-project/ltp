// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Check that a getitimer() call fails with EFAULT with invalid itimerval pointer.
 */

#include <errno.h>
#include <sys/time.h>
#include "tst_test.h"

static void verify_getitimer(void)
{
	TST_EXP_FAIL(getitimer(ITIMER_REAL, (struct itimerval *)-1), EFAULT);
}

static struct tst_test test = {
	.test_all = verify_getitimer,
};
