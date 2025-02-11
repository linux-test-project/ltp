// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 03/2001 - Written by Wayne Boyer
 *
 */

/*\
 * Check that setitimer() call fails:
 *
 * 1. EFAULT with invalid itimerval pointer
 * 2. EINVAL when called with an invalid first argument
 */

#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_timer.h"

static struct __kernel_old_itimerval *value, *ovalue;

static void verify_setitimer(unsigned int i)
{
	switch (i) {
	case 0:
		TST_EXP_FAIL(sys_setitimer(ITIMER_REAL, value, (void *)-1), EFAULT);
		break;
	case 1:
		TST_EXP_FAIL(sys_setitimer(ITIMER_VIRTUAL, value, (void *)-1), EFAULT);
		break;
	case 2:
		TST_EXP_FAIL(sys_setitimer(-ITIMER_PROF, value, ovalue), EINVAL);
		break;
	}
}

static void setup(void)
{
	value->it_value.tv_sec = 30;
	value->it_value.tv_usec = 0;
	value->it_interval.tv_sec = 0;
	value->it_interval.tv_usec = 0;
}

static struct tst_test test = {
	.tcnt = 3,
	.test = verify_setitimer,
	.setup = setup,
	.bufs = (struct tst_buffers[]) {
		{&value,  .size = sizeof(struct __kernel_old_itimerval)},
		{&ovalue, .size = sizeof(struct __kernel_old_itimerval)},
		{}
	}
};
