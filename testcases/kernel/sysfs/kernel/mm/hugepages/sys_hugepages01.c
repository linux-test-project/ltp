// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the hugepage pool attributes exported under
 * /sys/kernel/mm/hugepages/.
 *
 * Each supported hugepage size has its own hugepages-<size>kB directory with a
 * set of counters describing its pool. For each such directory the test
 * verifies that:
 *
 * - the size encoded in the directory name is a power of two (in kB)
 * - nr_hugepages, free_hugepages, resv_hugepages and surplus_hugepages are
 *   all non-negative
 * - free_hugepages <= nr_hugepages + surplus_hugepages (the free count cannot
 *   exceed the total pool)
 * - resv_hugepages <= free_hugepages (reserved pages are a subset of free)
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present.
 *
 * /sys/kernel/mm/hugepages/ itself may not exist even with CONFIG_HUGETLBFS=y:
 * hugetlb_init() in mm/hugetlb.c only calls hugetlb_sysfs_init() when
 * hugepages_supported() is true, and on powerpc that additionally requires
 * HPAGE_SHIFT != 0, i.e. hugetlbpage_init() having found at least one usable
 * hugepage size at boot.
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static int read_count(const char *dir, const char *attr, long *val)
{
	char path[PATH_MAX];

	snprintf(path, sizeof(path), PATH_MM_HUGEPAGES "/%s/%s", dir, attr);

	if (access(path, F_OK))
		return 1;

	if (FILE_SCANF(path, "%ld", val))
		return 1;

	return 0;
}

static void check_size(const char *dir)
{
	long size;

	if (sscanf(dir, "hugepages-%ldkB", &size) != 1) {
		tst_res(TFAIL, "Malformed hugepage directory name '%s'", dir);
		return;
	}

	if (size > 0 && !(size & (size - 1))) {
		tst_res(TPASS, "%s: size %ld kB is a power of two", dir, size);
	} else {
		tst_res(TFAIL, "%s: size %ld kB is not a power of two",
			dir, size);
	}
}

static void check_counts(const char *dir)
{
	long nr, free, resv, surplus;

	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_MM_HUGEPAGES "/%s/nr_hugepages", dir);
	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_MM_HUGEPAGES "/%s/free_hugepages", dir);
	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX, PATH_MM_HUGEPAGES "/%s/resv_hugepages", dir);
	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX,
			       PATH_MM_HUGEPAGES "/%s/surplus_hugepages", dir);

	if (read_count(dir, "nr_hugepages", &nr) ||
	    read_count(dir, "free_hugepages", &free) ||
	    read_count(dir, "surplus_hugepages", &surplus))
		return;

	if (free <= nr + surplus) {
		tst_res(TPASS,
			"%s: free_hugepages (%ld) <= nr_hugepages (%ld) + surplus_hugepages (%ld)",
			dir, free, nr, surplus);
	} else {
		tst_res(TFAIL,
			"%s: free_hugepages (%ld) > nr_hugepages (%ld) + surplus_hugepages (%ld)",
			dir, free, nr, surplus);
	}

	if (read_count(dir, "resv_hugepages", &resv))
		return;

	if (resv <= free) {
		tst_res(TPASS, "%s: resv_hugepages (%ld) <= free_hugepages (%ld)",
			dir, resv, free);
	} else {
		tst_res(TFAIL, "%s: resv_hugepages (%ld) > free_hugepages (%ld)",
			dir, resv, free);
	}
}

static void run(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;

	d = TST_SYSFS_TRY_OPENDIR(PATH_MM_HUGEPAGES);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "hugepages-", 10))
			continue;

		found = 1;
		tst_res(TINFO, "Checking hugepage pool '%s'", ent->d_name);
		check_size(ent->d_name);
		check_counts(ent->d_name);
	}

	SAFE_CLOSEDIR(d);

	if (!found)
		tst_res(TCONF, "No hugepage pools found");
}

static struct tst_test test = {
	.test_all = run,
	.needs_kconfigs = (const char *const[]){
		"CONFIG_HUGETLBFS=y",
		NULL
	},
};
