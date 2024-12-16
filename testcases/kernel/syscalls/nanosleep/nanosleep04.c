// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *
 * Ported to new library:
 * 07/2019    Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that nanosleep() returns -1 and sets errno to EINVAL when failing to suspend the
 * execution of a process if the specified pause time is invalid.
 */

#include <errno.h>
#include <time.h>
#include "tst_test.h"

static struct timespec tcases[] = {
	{.tv_sec = -5, .tv_nsec = 9999},
	{.tv_sec = 0, .tv_nsec = 1000000000},
	{.tv_sec = 1, .tv_nsec = -100},
};

static void verify_nanosleep(unsigned int n)
{
	TEST(nanosleep(&tcases[n], NULL));

	if (TST_RET != -1) {
		tst_res(TFAIL,
		        "nanosleep() returned %ld, expected -1", TST_RET);
		return;
	}

	if (TST_ERR != EINVAL) {
		tst_res(TFAIL | TTERRNO,
			"nanosleep() failed,expected EINVAL, got");
		return;
	}

	tst_res(TPASS, "nanosleep() failed with EINVAL");
}

static struct tst_test test = {
	.test = verify_nanosleep,
	.tcnt = ARRAY_SIZE(tcases),
};

