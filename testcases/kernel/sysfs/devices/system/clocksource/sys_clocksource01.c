// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the clocksource attributes exported under
 * /sys/devices/system/clocksource/clocksourceN/.
 *
 * available_clocksource lists all clocksources the kernel can switch to,
 * while current_clocksource names the one that is active. Unlike the
 * bracketed-choice files elsewhere in sysfs, the currently selected
 * clocksource is not marked inside available_clocksource, so the two files
 * have to be cross-checked explicitly. For every clocksourceN directory the
 * test verifies that:
 *
 * - current_clocksource is non-empty
 * - current_clocksource is one of the tokens listed in available_clocksource
 *
 * The test skips with TCONF when no clocksource directory is present.
 */

#include <string.h>
#include <dirent.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static void check_clocksource(const char *name)
{
	char current[64] = "";

	tst_res(TINFO, "Checking %s", name);

	TST_SYSFS_READ_STR(current, sizeof(current),
			   PATH_SYS_CLOCKSOURCE "/%s/current_clocksource", name);

	TST_SYSFS_ASSERT_TOKENS(NULL, current,
				PATH_SYS_CLOCKSOURCE "/%s/available_clocksource", name);
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;

	d = TST_SYSFS_TRY_OPENDIR(PATH_SYS_CLOCKSOURCE);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "clocksource", 11))
			continue;

		found = 1;
		check_clocksource(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!found)
		tst_res(TCONF, "No clocksource directory found");
}

static struct tst_test test = {
	.test_all = do_test,
};
