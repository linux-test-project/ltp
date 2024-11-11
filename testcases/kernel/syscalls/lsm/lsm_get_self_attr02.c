// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that lsm_get_self_attr syscall is acting correctly when ctx is NULL.
 * The syscall can behave in different ways according to the current system
 * status:
 *
 * - if any LSM is running inside the system, the syscall will pass and it will
 *   provide a size as big as the attribute
 * - if no LSM(s) are running inside the system, the syscall will fail with -1
 *   return code
 */
#include "lsm_common.h"

static uint32_t page_size;
static uint32_t lsm_count;

static void run(void)
{
	uint32_t size = page_size;

	if (lsm_count) {
		TST_EXP_POSITIVE(lsm_get_self_attr(
			LSM_ATTR_CURRENT, NULL, &size, 0));
		TST_EXP_EXPR(size > 1);
	} else {
		TST_EXP_FAIL(lsm_get_self_attr(
			LSM_ATTR_CURRENT, NULL, &size, 0), EOPNOTSUPP);
	}
}

static void setup(void)
{
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);
	lsm_count = count_supported_attr_current();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.min_kver = "6.8",
};
