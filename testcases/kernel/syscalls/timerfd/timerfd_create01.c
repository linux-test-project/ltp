// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 *         Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */
/*\
 * [Description]
 *
 * This test verifies that:
 * - clockid argument is neither CLOCK_MONOTONIC nor CLOCK_REALTIME,
 * EINVAL would return.
 * - flags is invalid, EINVAL would return.
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_safe_timerfd.h"

static struct test_case_t {
	int clockid;
	int flags;
	int exp_errno;
} tcases[] = {
	{ -1,  0, EINVAL },
	{  0, -1, EINVAL },
};

static void run(unsigned int i)
{
	struct test_case_t *test = &tcases[i];

	TST_EXP_FAIL(timerfd_create(test->clockid, test->flags), test->exp_errno);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
