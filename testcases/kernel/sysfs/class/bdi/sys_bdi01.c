// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the backing device info (BDI) attributes exported under
 * /sys/class/bdi/<id>/.
 *
 * For every BDI the test verifies that:
 *
 * - min_ratio <= max_ratio (both are percentages)
 * - min_ratio_fine <= max_ratio_fine (both are in parts-per-million, i.e.
 *   100% == 1000000)
 * - min_bytes <= max_bytes on kernel 6.2 or later, when max_bytes is set
 *   (0 means ``no limit``)
 * - stable_pages_required and strict_limit are booleans
 * - read_ahead_kb is non-negative
 *
 * The test skips with TCONF when no BDI is present.
 *
 * min_bytes/max_bytes were added by commit 1bf27e98d26d (``mm: add
 * bdi_set_max_bytes() function``) in kernel 6.2.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static int has_max_bytes;

static void check_bdi(const char *id)
{
	long long max_bytes;

	tst_res(TINFO, "Checking bdi '%s'", id);

	TST_SYSFS_ASSERT_RANGELL(0, 100, PATH_CLASS_BDI "/%s/min_ratio", id);
	TST_SYSFS_ASSERT_RANGELL(0, 100, PATH_CLASS_BDI "/%s/max_ratio", id);
	TST_SYSFS_ASSERT_LE_SUFFIX("min_ratio", "max_ratio",
				   PATH_CLASS_BDI "/%s/", id);

	TST_SYSFS_ASSERT_RANGELL(0, 1000000, PATH_CLASS_BDI "/%s/min_ratio_fine", id);
	TST_SYSFS_ASSERT_RANGELL(0, 1000000, PATH_CLASS_BDI "/%s/max_ratio_fine", id);
	TST_SYSFS_ASSERT_LE_SUFFIX("min_ratio_fine", "max_ratio_fine",
				   PATH_CLASS_BDI "/%s/", id);

	if (has_max_bytes) {
		max_bytes = TST_SYSFS_READ_LLI(PATH_CLASS_BDI "/%s/max_bytes", id);

		if (max_bytes > 0) {
			TST_SYSFS_ASSERT_LE_SUFFIX("min_bytes", "max_bytes",
						   PATH_CLASS_BDI "/%s/", id);
		}
	}

	TST_SYSFS_ASSERT_BOOL(PATH_CLASS_BDI "/%s/stable_pages_required", id);
	TST_SYSFS_ASSERT_BOOL(PATH_CLASS_BDI "/%s/strict_limit", id);

	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_CLASS_BDI "/%s/read_ahead_kb", id);
}

static void setup(void)
{
	has_max_bytes = tst_kvercmp(6, 2, 0) >= 0;
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;

	if (!tst_sysfs_exists(PATH_CLASS_BDI))
		tst_brk(TCONF, PATH_CLASS_BDI ": not present");

	d = SAFE_OPENDIR(PATH_CLASS_BDI);

	while ((ent = SAFE_READDIR(d))) {
		if (ent->d_name[0] == '.')
			continue;

		found = 1;
		check_bdi(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!found)
		tst_res(TCONF, "No BDI found");
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
};
