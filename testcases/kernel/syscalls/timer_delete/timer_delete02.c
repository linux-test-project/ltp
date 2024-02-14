// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
 * Author: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 * Ported to new library:
 * 07/2019      Christian Amann <camann@suse.com>
 */
/*
 * Basic error handling test for timer_delete(2):
 *
 *	This test case checks whether timer_delete(2) returns an appropriate
 *	error (EINVAL) for an invalid timerid parameter
 */

#include <errno.h>
#include <time.h>
#include "tst_test.h"
#include "lapi/common_timers.h"

#define INVALID_ID ((kernel_timer_t)-1)

static void run(void)
{
	TEST(tst_syscall(__NR_timer_delete, INVALID_ID));

	if (TST_RET == -1 && TST_ERR == EINVAL) {
		tst_res(TPASS | TTERRNO,
			 "Failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			 "Didn't fail with EINVAL as expected - got");
	}
}

static struct tst_test test = {
	.test_all = run,
};
