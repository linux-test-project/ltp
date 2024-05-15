// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that statmount() is working with no mask flags.
 *
 * [Algorithm]
 *
 * - create a mount point
 * - run statmount() on the mount point without giving any mask
 * - read results and check that mask and size are correct
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"

#define MNTPOINT "mntpoint"

static uint64_t mntpoint_id;
static struct statmount *st_mount;

static void run(void)
{
	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_PASS(statmount(mntpoint_id, 0, st_mount, sizeof(struct statmount), 0));

	if (!TST_PASS)
		return;

	TST_EXP_EQ_LI(st_mount->size, sizeof(struct statmount));
	TST_EXP_EQ_LI(st_mount->mask, 0);
	TST_EXP_EQ_LI(st_mount->sb_dev_major, 0);
	TST_EXP_EQ_LI(st_mount->sb_dev_minor, 0);
	TST_EXP_EQ_LI(st_mount->sb_magic, 0);
	TST_EXP_EQ_LI(st_mount->sb_flags, 0);
	TST_EXP_EQ_LI(st_mount->fs_type, 0);
	TST_EXP_EQ_LI(st_mount->mnt_id, 0);
	TST_EXP_EQ_LI(st_mount->mnt_parent_id, 0);
	TST_EXP_EQ_LI(st_mount->mnt_id_old, 0);
	TST_EXP_EQ_LI(st_mount->mnt_parent_id_old, 0);
	TST_EXP_EQ_LI(st_mount->mnt_attr, 0);
	TST_EXP_EQ_LI(st_mount->mnt_propagation, 0);
	TST_EXP_EQ_LI(st_mount->mnt_peer_group, 0);
	TST_EXP_EQ_LI(st_mount->mnt_master, 0);
	TST_EXP_EQ_LI(st_mount->propagate_from, 0);
	TST_EXP_EQ_LI(st_mount->mnt_root, 0);
	TST_EXP_EQ_LI(st_mount->mnt_point, 0);
}

static void setup(void)
{
	struct ltp_statx sx;

	SAFE_STATX(AT_FDCWD, MNTPOINT, 0, STATX_MNT_ID_UNIQUE, &sx);

	mntpoint_id = sx.data.stx_mnt_id;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{}
	}
};
