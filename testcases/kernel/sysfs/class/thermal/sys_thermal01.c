// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the thermal cooling devices exported under
 * /sys/class/thermal/cooling_deviceN/.
 *
 * For every cooling device the test verifies that:
 *
 * - type is a non-empty string
 * - max_state is non-negative
 * - cur_state is in the range [0, max_state]
 *
 * The test skips with TCONF when no cooling device is present.
 */

#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static void check_cooling_device(const char *name)
{
	char type[64] = "";

	tst_res(TINFO, "Checking %s", name);

	TST_SYSFS_READ_STR(type, sizeof(type), PATH_CLASS_THERMAL "/%s/type", name);

	if (type[0] != '\0')
		tst_res(TPASS, "%s: type = '%s'", name, type);
	else
		tst_res(TFAIL, "%s: type is empty", name);

	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_CLASS_THERMAL "/%s/max_state", name);

	TST_SYSFS_ASSERT_RANGELF(0, "cur_state", "max_state",
				  PATH_CLASS_THERMAL "/%s/", name);
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;

	if (!tst_sysfs_exists(PATH_CLASS_THERMAL))
		tst_brk(TCONF, PATH_CLASS_THERMAL ": not present");

	d = SAFE_OPENDIR(PATH_CLASS_THERMAL);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "cooling_device", 14))
			continue;

		found = 1;
		check_cooling_device(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!found)
		tst_res(TCONF, "No cooling device found");
}

static struct tst_test test = {
	.test_all = do_test,
};
