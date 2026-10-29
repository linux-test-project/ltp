// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity check for /sys/kernel/mm/swap/vma_ra_enabled.
 *
 * Unlike most other boolean sysfs tunables which use 0/1, this one is
 * formatted as the strings ``true`` or ``false``, so it needs its own check
 * rather than TST_SYSFS_ASSERT_BOOL().
 *
 * The test skips with TCONF when the attribute is not present.
 */

#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const bool_str_allowed[] = {
	"true", "false", NULL
};

static void do_test(void)
{
	TST_SYSFS_ASSERT_ONEOF(bool_str_allowed, PATH_MM_SWAP "/vma_ra_enabled");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_SWAP=y",
		NULL
	},
};
