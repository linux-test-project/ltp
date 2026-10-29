// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Cross-checks the default hugepage pool in sysfs against /proc/meminfo.
 *
 * /proc/meminfo describes the default hugepage size pool, which corresponds to
 * one of the hugepages-<size>kB directories under /sys/kernel/mm/hugepages/.
 * The test verifies that for the default pool:
 *
 * - Hugepagesize in /proc/meminfo matches the default pool directory size
 * - HugePages_Total matches nr_hugepages
 * - HugePages_Free matches free_hugepages
 * - HugePages_Rsvd matches resv_hugepages
 * - HugePages_Surp matches surplus_hugepages
 *
 * The test skips with TCONF when hugepages are not available.
 */

#include <stdio.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static long hugepagesize;

static void compare(const char *what, const char *meminfo_key, const char *attr,
		    long hugepagesize)
{
	char path[PATH_MAX];
	long meminfo_val, sysfs_val;

	SAFE_FILE_LINES_SCANF("/proc/meminfo", meminfo_key, &meminfo_val);

	snprintf(path, sizeof(path), PATH_MM_HUGEPAGES "/hugepages-%ldkB/%s",
		 hugepagesize, attr);
	SAFE_FILE_SCANF(path, "%ld", &sysfs_val);

	if (meminfo_val == sysfs_val) {
		tst_res(TPASS, "%s: /proc/meminfo (%ld) == %s (%ld)",
			what, meminfo_val, attr, sysfs_val);
	} else {
		tst_res(TFAIL, "%s: /proc/meminfo (%ld) != %s (%ld)",
			what, meminfo_val, attr, sysfs_val);
	}
}

static void do_test(void)
{
	if (FILE_LINES_SCANF("/proc/meminfo", MEMINFO_HPAGE_SIZE " %ld",
			     &hugepagesize)) {
		tst_res(TCONF, "No Hugepagesize in /proc/meminfo");
		return;
	}

	if (!tst_sysfs_exists(PATH_MM_HUGEPAGES "/hugepages-%ldkB", hugepagesize)) {
		tst_res(TCONF,
			"Default hugepage pool hugepages-%ldkB not found in sysfs",
			hugepagesize);
		return;
	}

	tst_res(TPASS,
		"/proc/meminfo Hugepagesize (%ld kB) has a matching sysfs pool",
		hugepagesize);

	compare("HugePages_Total", MEMINFO_HPAGE_TOTAL " %ld",
		"nr_hugepages", hugepagesize);
	compare("HugePages_Free", MEMINFO_HPAGE_FREE " %ld",
		"free_hugepages", hugepagesize);
	compare("HugePages_Rsvd", MEMINFO_HPAGE_RSVD " %ld",
		"resv_hugepages", hugepagesize);
	compare("HugePages_Surp", MEMINFO_HPAGE_SURP " %ld",
		"surplus_hugepages", hugepagesize);
}

static void setup(void)
{
	if (FILE_LINES_SCANF("/proc/meminfo", MEMINFO_HPAGE_SIZE " %ld",
			     &hugepagesize)) {
		tst_brk(TCONF, "No Hugepagesize in /proc/meminfo");
	}

	if (!tst_sysfs_exists(PATH_MM_HUGEPAGES "/hugepages-%ldkB", hugepagesize)) {
		tst_brk(TCONF,
			"Default hugepage pool hugepages-%ldkB not found in sysfs",
			hugepagesize);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_test,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_HUGETLBFS=y",
		NULL
	},
};
