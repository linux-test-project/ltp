// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the CPU topology attributes exported under
 * /sys/devices/system/cpu/.
 *
 * The kernel exports several cpulist files describing the state of the CPUs:
 *
 * - possible - CPUs that could possibly be brought online during this boot
 * - present  - CPUs that are populated (physically present)
 * - online   - CPUs that are currently online and usable
 * - offline  - CPUs that are known to the system but currently offline
 *
 * These sets are nested, so the test verifies that:
 *
 * - online is a subset of present
 * - present is a subset of possible
 * - offline is a subset of possible
 * - kernel_max is greater than or equal to the highest possible CPU id
 *
 * The number of online CPUs is also cross-checked against /proc/stat, which
 * contains one ``cpuN`` line for each online CPU.
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static int count_proc_stat_cpus(void)
{
	FILE *f;
	char line[4096];
	int cpus = 0;

	f = SAFE_FOPEN("/proc/stat", "r");

	while (fgets(line, sizeof(line), f)) {
		if (!strncmp(line, "cpu", 3) && isdigit(line[3]))
			cpus++;
	}

	SAFE_FCLOSE(f);

	return cpus;
}

static void check_online_proc_stat(void)
{
	int count, max_id;
	int stat_cpus;

	if (TST_SYSFS_ASSERT_PARSE_LIST(&count, &max_id, PATH_SYS_CPU "/online"))
		return;

	stat_cpus = count_proc_stat_cpus();

	if (count == stat_cpus) {
		tst_res(TPASS,
			PATH_SYS_CPU "/online count (%d) == /proc/stat cpu lines (%d)",
			count, stat_cpus);
	} else {
		tst_res(TFAIL,
			PATH_SYS_CPU "/online count (%d) != /proc/stat cpu lines (%d)",
			count, stat_cpus);
	}
}

static void check_kernel_max(void)
{
	int count, max_id;

	if (TST_SYSFS_ASSERT_PARSE_LIST(&count, &max_id, PATH_SYS_CPU "/possible"))
		return;

	TST_SYSFS_ASSERT_RANGELL(max_id, LONG_MAX, PATH_SYS_CPU "/kernel_max");
}

static void run(void)
{
	TST_SYSFS_ASSERT_LIST_SUBSET(PATH_SYS_CPU "/online", PATH_SYS_CPU "/present");
	TST_SYSFS_ASSERT_LIST_SUBSET(PATH_SYS_CPU "/present", PATH_SYS_CPU "/possible");
	TST_SYSFS_ASSERT_LIST_SUBSET(PATH_SYS_CPU "/offline", PATH_SYS_CPU "/possible");

	check_kernel_max();

	check_online_proc_stat();
}

static struct tst_test test = {
	.test_all = run,
};
