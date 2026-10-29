// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the NUMA node attributes exported under
 * /sys/devices/system/node/.
 *
 * The kernel exports cpulist-style files describing the node state:
 *
 * - possible - node ids that could possibly be online
 * - online   - node ids that are currently online
 *
 * as well as a per-node memory summary in nodeN/meminfo.
 *
 * The test verifies that:
 *
 * - online is a subset of possible
 * - for each online node MemUsed == MemTotal - MemFree
 * - the sum of all per-node MemTotal values matches MemTotal in /proc/meminfo
 *
 * All checks skip gracefully with TCONF on systems without NUMA sysfs.
 *
 * The last check is only enforced (TFAIL on mismatch) on physical hardware.
 * /proc/meminfo's MemTotal (si_meminfo()'s totalram_pages()) and a node's
 * MemTotal (si_meminfo_node()'s sum of that node's zones' managed pages) are
 * two independently maintained counters, normally kept in lockstep by
 * adjust_managed_page_count() updating both together, so their sums match
 * exactly on real hardware. Under virtualization, memory hotplug/ballooning
 * mechanisms (e.g. virtio-mem, virtio-balloon, Xen balloon) are a known
 * source of drift between the two, since they can affect one counter without
 * (yet) correspondingly attributing the change to a specific NUMA node's
 * zones, so a mismatch there is only reported as TINFO.
 */

#include <stdio.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static long read_node_meminfo(int nid, const char *item)
{
	char path[PATH_MAX];
	char fmt[128];
	long val;

	snprintf(path, sizeof(path), PATH_SYS_NODE "/node%d/meminfo", nid);
	snprintf(fmt, sizeof(fmt), "Node %d %s: %%ld", nid, item);

	SAFE_FILE_LINES_SCANF(path, fmt, &val);

	return val;
}

static void check_node_mem(int nid, long *sum_total)
{
	long total, free, used;

	total = read_node_meminfo(nid, "MemTotal");
	free = read_node_meminfo(nid, "MemFree");
	used = read_node_meminfo(nid, "MemUsed");

	*sum_total += total;

	if (used == total - free) {
		tst_res(TPASS,
			"node%d: MemUsed (%ld) == MemTotal (%ld) - MemFree (%ld)",
			nid, used, total, free);
	} else {
		tst_res(TFAIL,
			"node%d: MemUsed (%ld) != MemTotal (%ld) - MemFree (%ld)",
			nid, used, total, free);
	}
}

static void check_sum_memtotal(long sum_total)
{
	long global_total;

	SAFE_FILE_LINES_SCANF("/proc/meminfo", "MemTotal: %ld", &global_total);

	if (sum_total == global_total) {
		tst_res(TPASS,
			"sum of node MemTotal (%ld kB) matches /proc/meminfo MemTotal (%ld kB)",
			sum_total, global_total);
		return;
	}

	if (tst_is_virt(VIRT_ANY)) {
		tst_res(TINFO,
			"Running in a VM, sum of node MemTotal (%ld kB) differs from /proc/meminfo MemTotal (%ld kB), likely memory hotplug/ballooning",
			sum_total, global_total);
	} else {
		tst_res(TFAIL,
			"sum of node MemTotal (%ld kB) differs from /proc/meminfo MemTotal (%ld kB)",
			sum_total, global_total);
	}
}

static void run(void)
{
	int count, max_id, nid;
	long sum_total = 0;

	if (access(PATH_SYS_NODE "/online", F_OK)) {
		tst_res(TCONF, PATH_SYS_NODE " is not available");
		return;
	}

	TST_SYSFS_ASSERT_LIST_SUBSET(PATH_SYS_NODE "/online", PATH_SYS_NODE "/possible");

	if (TST_SYSFS_ASSERT_PARSE_LIST(&count, &max_id, PATH_SYS_NODE "/online"))
		return;

	for (nid = 0; nid <= max_id; nid++) {
		if (!tst_sysfs_exists(PATH_SYS_NODE "/node%d", nid))
			continue;

		check_node_mem(nid, &sum_total);
	}

	check_sum_memtotal(sum_total);
}

static struct tst_test test = {
	.test_all = run,
};
