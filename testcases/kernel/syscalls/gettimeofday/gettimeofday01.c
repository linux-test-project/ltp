// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2001-2023
 */

/*\
 * Test for gettimeofday error.
 *
 * - EFAULT: tv pointed outside the accessible address space
 * - EFAULT: tz pointed outside the accessible address space
 * - EFAULT: both tv and tz pointed outside the accessible address space
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static struct timeval tv1;

static struct tcase {
	void *tv;
	void *tz;
} tcases[] = {
	/* timezone structure is obsolete, tz should be treated as null */
	{(void *)-1, NULL},
	{&tv1, (void *)-1},
	{(void *)-1, (void *)-1},
};

static void verify_gettimeofday(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(tst_syscall(__NR_gettimeofday, tc->tv, tc->tz), EFAULT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_gettimeofday,
};
