// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Cross-checks the block device size reported in sysfs against /proc/partitions.
 *
 * /sys/block/<dev>/size holds the device size in 512 byte sectors while
 * /proc/partitions lists the size in 1024 byte blocks. For every whole block
 * device the test verifies that:
 *
 * - the two sources agree, i.e. sysfs_size_in_sectors / 2 == proc_blocks
 * - the ro (read-only) attribute is a boolean
 *
 * The test skips with TCONF when a device is not present in /proc/partitions
 * (for example device-mapper devices that are only listed under /sys/block).
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static int proc_partition_blocks(const char *dev, long long *blocks)
{
	FILE *f;
	char line[4096];
	int found = 0;

	f = SAFE_FOPEN("/proc/partitions", "r");

	while (fgets(line, sizeof(line), f)) {
		long long nblocks;
		char name[256];

		if (sscanf(line, "%*u %*u %lld %255s", &nblocks, name) != 2)
			continue;

		if (!strcmp(name, dev)) {
			*blocks = nblocks;
			found = 1;
			break;
		}
	}

	SAFE_FCLOSE(f);

	return found;
}

static void check_device(const char *dev)
{
	long long sectors, proc_blocks;

	tst_res(TINFO, "Checking block device '%s'", dev);

	TST_SYSFS_ASSERT_BOOL(PATH_SYS_BLOCK "/%s/ro", dev);

	if (!proc_partition_blocks(dev, &proc_blocks)) {
		tst_res(TCONF, "%s not listed in /proc/partitions", dev);
		return;
	}

	sectors = TST_SYSFS_READ_LLI(PATH_SYS_BLOCK "/%s/size", dev);

	/* sysfs size is in 512 byte sectors, /proc/partitions in 1024 byte blocks */
	if (sectors / 2 == proc_blocks) {
		tst_res(TPASS,
			"%s: sysfs size %lld sectors matches /proc/partitions %lld blocks",
			dev, sectors, proc_blocks);
	} else {
		tst_res(TFAIL,
			"%s: sysfs size %lld sectors (/2 = %lld) != /proc/partitions %lld blocks",
			dev, sectors, sectors / 2, proc_blocks);
	}
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;

	d = TST_SYSFS_TRY_OPENDIR(PATH_SYS_BLOCK);

	while ((ent = SAFE_READDIR(d))) {
		if (ent->d_name[0] == '.')
			continue;

		check_device(ent->d_name);
	}

	SAFE_CLOSEDIR(d);
}

static struct tst_test test = {
	.test_all = do_test,
};
