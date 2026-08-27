// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the Transparent HugePage (THP) attributes exported under
 * /sys/kernel/mm/transparent_hugepage/.
 *
 * The test verifies that:
 *
 * - enabled is a bracketed-choice file selecting one of always/madvise/never
 * - defrag selects one of always/defer/defer+madvise/madvise/never
 * - shmem_enabled selects one of always/within_size/advise/never/deny/force
 * - use_zero_page is a boolean
 * - hpage_pmd_size is a power of two and larger than the base page size
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present, since the set of THP tunables differs between kernel versions.
 */

#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const enabled_allowed[] = {
	"always", "madvise", "never", NULL
};

static const char *const defrag_allowed[] = {
	"always", "defer", "defer+madvise", "madvise", "never", NULL
};

static const char *const shmem_allowed[] = {
	"always", "within_size", "advise", "never", "deny", "force", NULL
};

static void check_hpage_pmd_size(void)
{
	long size, pagesize;

	if (access(PATH_MM_THP "/hpage_pmd_size", F_OK)) {
		tst_res(TCONF, PATH_MM_THP "/hpage_pmd_size does not exist");
		return;
	}

	TST_SYSFS_ASSERT_POW2(PATH_MM_THP "/hpage_pmd_size");

	size = TST_SYSFS_READ_LI(PATH_MM_THP "/hpage_pmd_size");
	pagesize = getpagesize();

	if (size > pagesize) {
		tst_res(TPASS, "hpage_pmd_size (%ld) > page size (%ld)",
			size, pagesize);
	} else {
		tst_res(TFAIL, "hpage_pmd_size (%ld) <= page size (%ld)",
			size, pagesize);
	}
}

static void run(void)
{
	char sel[32];

	TST_SYSFS_ASSERT_CHOICE(enabled_allowed, sel, sizeof(sel),
				PATH_MM_THP "/enabled");
	TST_SYSFS_ASSERT_CHOICE(defrag_allowed, sel, sizeof(sel),
				PATH_MM_THP "/defrag");
	TST_SYSFS_ASSERT_CHOICE(shmem_allowed, sel, sizeof(sel),
				PATH_MM_THP "/shmem_enabled");

	TST_SYSFS_ASSERT_BOOL(PATH_MM_THP "/use_zero_page");

	check_hpage_pmd_size();
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_TRANSPARENT_HUGEPAGE=y",
		NULL
	},
};
