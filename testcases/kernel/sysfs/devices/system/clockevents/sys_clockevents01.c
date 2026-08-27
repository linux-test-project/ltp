// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the per-CPU clock event devices exported under
 * /sys/devices/system/clockevents/clockeventN/.
 *
 * The kernel's tick_init_sysfs() creates one clockeventN directory for every
 * *possible* CPU (for_each_possible_cpu()), not just the online ones, and N
 * is the CPU id the directory belongs to (broadcast is a separate, extra
 * entry used for CPUs whose local timer stops in idle). These directories
 * are never removed on CPU hotplug, so a clockeventN directory exists
 * regardless of whether cpuN is currently online.
 *
 * However current_device only names a device while the CPU is online:
 * tick_shutdown(), called while taking a CPU offline, clears the per-CPU
 * evtdev pointer, which makes current_device_show() emit an empty string.
 * So current_device is expected to be non-empty only for online CPUs, it is
 * legitimately empty for offline (or not-present) ones.
 *
 * On platforms without a per-CPU timer, secondary CPUs may instead rely
 * entirely on a broadcast IPI from a single shared timer without ever
 * getting their own clockeventN entry, so the number of clockeventN
 * directories is only asserted not to exceed the number of possible CPUs
 * rather than to match it exactly. The test verifies that:
 *
 * - there is at least one clockeventN directory
 * - the number of clockeventN directories does not exceed the number of
 *   possible CPUs
 * - each clockeventN/current_device names a non-empty clock event device
 *   while cpuN is online, and is not checked while cpuN is offline or not
 *   present
 *
 * The test skips with TCONF when no clockevents directory is present.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static int cpu_is_online(int cpu_id)
{
	char online[8];

	/* CPU not present at all, e.g. possible but never plugged in. */
	if (!tst_sysfs_exists(PATH_SYS_CPU "/cpu%d", cpu_id))
		return 0;

	/*
	 * Some CPUs, e.g. the boot CPU on some arches, cannot be offlined
	 * and therefore have no "online" file. Such a CPU is always online.
	 */
	if (!TST_SYSFS_READ_STR(online, sizeof(online),
				PATH_SYS_CPU "/cpu%d/online", cpu_id))
		return 1;

	return atoi(online) == 1;
}

static void check_clockevent(const char *name)
{
	char device[64] = "";
	int cpu_id, online;

	if (sscanf(name, "clockevent%d", &cpu_id) != 1) {
		tst_res(TWARN, "Cannot parse CPU id out of '%s'", name);
		return;
	}

	online = cpu_is_online(cpu_id);

	TST_SYSFS_READ_STR(device, sizeof(device),
			   PATH_SYS_CLOCKEVENTS "/%s/current_device", name);

	if (device[0] != '\0') {
		tst_res(TPASS, "%s/current_device = '%s'", name, device);
		return;
	}

	if (online) {
		tst_res(TFAIL,
			"%s/current_device is empty while cpu%d is online",
			name, cpu_id);
	} else {
		tst_res(TPASS,
			"%s/current_device is empty while cpu%d is offline",
			name, cpu_id);
	}
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int nclockevents = 0;
	int possible_count, max_id;

	d = TST_SYSFS_TRY_OPENDIR(PATH_SYS_CLOCKEVENTS);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "clockevent", 10))
			continue;

		nclockevents++;
		check_clockevent(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!nclockevents) {
		tst_res(TCONF, "No clockevent directory found");
		return;
	}

	if (TST_SYSFS_ASSERT_PARSE_LIST(&possible_count, &max_id,
					PATH_SYS_CPU "/possible"))
		return;

	if (nclockevents <= possible_count) {
		tst_res(TPASS,
			"number of clockevent devices (%d) does not exceed possible CPUs (%d)",
			nclockevents, possible_count);
	} else {
		tst_res(TFAIL,
			"number of clockevent devices (%d) exceeds possible CPUs (%d)",
			nclockevents, possible_count);
	}
}

static struct tst_test test = {
	.test_all = do_test,
};
