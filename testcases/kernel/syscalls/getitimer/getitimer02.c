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
#include "lapi/syscalls.h"

static int sys_getitimer(int which, void *curr_value)
{
	return tst_syscall(__NR_getitimer, which, curr_value);
}

static void verify_getitimer(void)
{
	TST_EXP_FAIL(sys_getitimer(ITIMER_REAL, (struct itimerval *)-1), EFAULT);
}

static struct tst_test test = {
	.test_all = verify_getitimer,
};
