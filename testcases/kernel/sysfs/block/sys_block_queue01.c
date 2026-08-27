// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the per block device request queue attributes exported
 * under /sys/block/<dev>/queue/.
 *
 * For every block device that exports a request queue the test verifies that:
 *
 * - logical_block_size and physical_block_size are powers of two
 * - logical_block_size <= physical_block_size
 * - hw_sector_size equals logical_block_size
 * - max_sectors_kb <= max_hw_sectors_kb
 * - read_ahead_kb, nr_requests and discard_max_bytes are non-negative
 * - nr_requests is greater than zero
 * - rotational, add_random, iostats are boolean (0 or 1)
 * - scheduler is a valid bracketed-choice file with exactly one active scheduler
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present, since the set of exported queue attributes differs between kernel
 * versions and device types.
 */

#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

/* schedulers that have existed in mainline over time */
static const char *const sched_allowed[] = {
	"none", "noop", "deadline", "mq-deadline", "cfq",
	"kyber", "bfq", "anticipatory", NULL
};

static void check_cross(const char *dev)
{
	TST_SYSFS_ASSERT_LE_SUFFIX("logical_block_size", "physical_block_size",
				   PATH_SYS_BLOCK "/%s/queue/", dev);

	TST_SYSFS_ASSERT_EQ_SUFFIX("logical_block_size", "hw_sector_size",
				   PATH_SYS_BLOCK "/%s/queue/", dev);

	TST_SYSFS_ASSERT_LE_SUFFIX("max_sectors_kb", "max_hw_sectors_kb",
				   PATH_SYS_BLOCK "/%s/queue/", dev);
}

static void check_device(const char *dev)
{
	char sched[64];

	tst_res(TINFO, "Checking block device '%s'", dev);

	TST_SYSFS_ASSERT_POW2(PATH_SYS_BLOCK "/%s/queue/logical_block_size", dev);
	TST_SYSFS_ASSERT_POW2(PATH_SYS_BLOCK "/%s/queue/physical_block_size", dev);

	TST_SYSFS_ASSERT_RANGELL(0, LONG_MAX,
			       PATH_SYS_BLOCK "/%s/queue/read_ahead_kb", dev);
	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX,
			       PATH_SYS_BLOCK "/%s/queue/nr_requests", dev);
	TST_SYSFS_ASSERT_RANGEUU(0, ULLONG_MAX,
			       PATH_SYS_BLOCK "/%s/queue/discard_max_bytes", dev);

	TST_SYSFS_ASSERT_BOOL(PATH_SYS_BLOCK "/%s/queue/rotational", dev);
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_BLOCK "/%s/queue/add_random", dev);
	TST_SYSFS_ASSERT_BOOL(PATH_SYS_BLOCK "/%s/queue/iostats", dev);

	TST_SYSFS_ASSERT_CHOICE(sched_allowed, sched, sizeof(sched),
				PATH_SYS_BLOCK "/%s/queue/scheduler", dev);

	check_cross(dev);
}

static void run(void)
{
	DIR *dir;
	struct dirent *ent;
	int found = 0;

	dir = TST_SYSFS_TRY_OPENDIR(PATH_SYS_BLOCK);

	while ((ent = SAFE_READDIR(dir))) {
		if (ent->d_name[0] == '.')
			continue;

		/* Only devices that export a request queue are of interest. */
		if (!tst_sysfs_exists(PATH_SYS_BLOCK "/%s/queue", ent->d_name))
			continue;

		found = 1;
		check_device(ent->d_name);
	}

	SAFE_CLOSEDIR(dir);

	if (!found)
		tst_res(TCONF, "No block device with a request queue found");
}

static struct tst_test test = {
	.test_all = run,
};
