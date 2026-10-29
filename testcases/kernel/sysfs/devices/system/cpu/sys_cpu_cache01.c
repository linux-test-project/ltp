// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the CPU cache attributes exported under
 * /sys/devices/system/cpu/cpuN/cache/indexM/.
 *
 * For every cache index of every online CPU the test verifies that:
 *
 * - level is greater than zero
 * - type is one of Data, Instruction or Unified
 * - coherency_line_size is a power of two
 * - number_of_sets and ways_of_associativity are greater than zero
 * - shared_cpu_list is a subset of the online CPUs
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static const char *const type_allowed[] = {
	"Data", "Instruction", "Unified", NULL
};

static void check_index(int cpu, const char *index)
{
	char shared_cpu_list[PATH_MAX];

	tst_res(TINFO, "Checking cpu%d %s", cpu, index);

	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX,
			       PATH_SYS_CPU "/cpu%d/cache/%s/level", cpu, index);

	TST_SYSFS_ASSERT_ONEOF(type_allowed,
			       PATH_SYS_CPU "/cpu%d/cache/%s/type", cpu, index);

	TST_SYSFS_ASSERT_POW2(PATH_SYS_CPU "/cpu%d/cache/%s/coherency_line_size",
			      cpu, index);

	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX,
			       PATH_SYS_CPU "/cpu%d/cache/%s/number_of_sets",
			       cpu, index);

	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX,
			       PATH_SYS_CPU "/cpu%d/cache/%s/ways_of_associativity",
			       cpu, index);

	snprintf(shared_cpu_list, sizeof(shared_cpu_list),
		 PATH_SYS_CPU "/cpu%d/cache/%s/shared_cpu_list", cpu, index);
	TST_SYSFS_ASSERT_LIST_SUBSET(shared_cpu_list, PATH_SYS_CPU "/online");
}

static int check_cpu_caches(int cpu)
{
	char cache_dir[PATH_MAX];
	DIR *d;
	struct dirent *ent;
	int found = 0;

	snprintf(cache_dir, sizeof(cache_dir), PATH_SYS_CPU "/cpu%d/cache", cpu);

	if (access(cache_dir, F_OK))
		return 0;

	d = SAFE_OPENDIR(cache_dir);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "index", 5))
			continue;

		found = 1;
		check_index(cpu, ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	return found;
}

static void do_test(void)
{
	int count, max_id, cpu;
	int found = 0;

	if (TST_SYSFS_ASSERT_PARSE_LIST(&count, &max_id, PATH_SYS_CPU "/online"))
		return;

	for (cpu = 0; cpu <= max_id; cpu++)
		found |= check_cpu_caches(cpu);

	if (!found)
		tst_res(TCONF, "No CPU cache info found");
}

static struct tst_test test = {
	.test_all = do_test,
};
