// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the wakeup source statistics exported under
 * /sys/class/wakeup/wakeupN/.
 *
 * For every wakeup source the test verifies that:
 *
 * - name is a non-empty string
 * - active_count, active_time_ms, event_count, expire_count, last_change_ms,
 *   max_time_ms, prevent_suspend_time_ms, total_time_ms and wakeup_count are
 *   all non-negative
 * - active_time_ms <= total_time_ms (currently active time is part of the
 *   accumulated total)
 * - max_time_ms <= total_time_ms (the longest single event cannot exceed the
 *   accumulated total)
 *
 * The test skips with TCONF when no wakeup source is present.
 *
 * The /sys/class/wakeup/ itself only exists when CONFIG_PM_SLEEP=y is set.
 */

#include <string.h>
#include <limits.h>
#include <dirent.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

/*
 * active_count/event_count/wakeup_count/expire_count are exported by the
 * kernel as "unsigned long" (see drivers/base/power/wakeup_stats.c), hence
 * the RANGEUU (unsigned long long bounds) variant is used for these: it
 * lets us use the real ULONG_MAX.
 */
static const char *const count_attrs[] = {
	"active_count",
	"event_count",
	"expire_count",
	"wakeup_count",
};

/*
 * active_time_ms/total_time_ms/max_time_ms/last_change_ms/
 * prevent_suspend_time_ms are exported by the kernel as a signed 64bit
 * ktime_t converted to milliseconds (see drivers/base/power/wakeup_stats.c),
 * i.e. their real ceiling is LLONG_MAX regardless of the architecture. On a
 * 32bit test build "long" is only 32bit (~24.8 days worth of ms), so plain
 * LONG_MAX would be too tight and could false-fail on a long-uptime system.
 */
static const char *const time_ms_attrs[] = {
	"active_time_ms",
	"last_change_ms",
	"max_time_ms",
	"prevent_suspend_time_ms",
	"total_time_ms",
};

static void check_wakeup(const char *dev)
{
	char name[128] = "";
	unsigned int i;

	tst_res(TINFO, "Checking %s", dev);

	if (!TST_SYSFS_READ_STR(name, sizeof(name), PATH_CLASS_WAKEUP "/%s/name", dev)) {
		tst_res(TCONF, "%s vanished before it could be checked", dev);
		return;
	}

	if (name[0] != '\0')
		tst_res(TPASS, "%s: name = '%s'", dev, name);
	else
		tst_res(TFAIL, "%s: name is empty", dev);

	for (i = 0; i < ARRAY_SIZE(count_attrs); i++) {
		TST_SYSFS_ASSERT_RANGEUU(0, ULONG_MAX, PATH_CLASS_WAKEUP "/%s/%s", dev,
					 count_attrs[i]);
	}

	for (i = 0; i < ARRAY_SIZE(time_ms_attrs); i++) {
		TST_SYSFS_ASSERT_RANGELL(0, LLONG_MAX, PATH_CLASS_WAKEUP "/%s/%s", dev,
					 time_ms_attrs[i]);
	}

	TST_SYSFS_ASSERT_LE_SUFFIX("active_time_ms", "total_time_ms",
				   PATH_CLASS_WAKEUP "/%s/", dev);
	TST_SYSFS_ASSERT_LE_SUFFIX("max_time_ms", "total_time_ms",
				   PATH_CLASS_WAKEUP "/%s/", dev);
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;

	d = TST_SYSFS_TRY_OPENDIR(PATH_CLASS_WAKEUP);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "wakeup", 6))
			continue;

		found = 1;
		check_wakeup(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!found)
		tst_res(TCONF, "No wakeup source found");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_kconfigs = (const char *[]){
		"CONFIG_PM_SLEEP=y",
		NULL
	},
};
