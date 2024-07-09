// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2024
 * Author: Ma Xinjian <maxj.fnst@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that sigsuspend(2) fails with
 *
 * - EFAULT mask points to memory which is not a valid part of the
 *          process address space.
 */

#include "tst_test.h"

static void *invalid_mask;

static void setup(void)
{
	invalid_mask = tst_get_bad_addr(NULL);
}

static void verify_sigsuspend(void)
{
	TST_EXP_FAIL(sigsuspend(invalid_mask), EFAULT);
}

static struct tst_test test = {
	.test_all = verify_sigsuspend,
	.setup = setup,
};
