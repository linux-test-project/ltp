// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 *
 */

/*\
 * [Description]
 *
 * Check that a setitimer() call fails with EFAULT with invalid itimerval
 * pointer.
 */

#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tst_test.h"

static struct itimerval *value;

static void verify_setitimer(void)
{
	TST_EXP_FAIL(setitimer(ITIMER_REAL, value, (struct itimerval *)-1),
	             EFAULT);
}

static void setup(void)
{
	value->it_value.tv_sec = 30;
	value->it_value.tv_usec = 0;
	value->it_interval.tv_sec = 0;
	value->it_interval.tv_usec = 0;
}

static struct tst_test test = {
	.test_all = verify_setitimer,
	.setup = setup,
	.bufs = (struct tst_buffers[]) {
		{&value, .size = sizeof(struct itimerval)},
		{}
	}
};
