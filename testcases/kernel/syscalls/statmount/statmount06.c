// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that statmount() is correctly reading name of the
 * filesystem type using STATMOUNT_FS_TYPE.
 *
 * [Algorithm]
 *
 * - create a mount point
 * - run statmount() on the mount point using STATMOUNT_FS_TYPE
 * - read results and check if contain the name of the filesystem
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"

#define MNTPOINT "mntpoint"
#define SM_SIZE (1 << 10)

static uint64_t root_id;
static struct statmount *st_mount;

static void run(void)
{
	memset(st_mount, 0, SM_SIZE);

	TST_EXP_PASS(statmount(root_id,	STATMOUNT_FS_TYPE, st_mount,
		SM_SIZE, 0));

	if (!TST_PASS)
		return;

	const char *fs_type = tst_device->is_fuse ? "fuseblk" : tst_device->fs_type;

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_FS_TYPE);
	TST_EXP_EQ_STR(st_mount->str + st_mount->fs_type, fs_type);
}

static void setup(void)
{
	struct ltp_statx sx;

	SAFE_STATX(AT_FDCWD, MNTPOINT, 0, STATX_MNT_ID_UNIQUE, &sx);
	root_id = sx.data.stx_mnt_id;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = SM_SIZE},
		{}
	}
};
