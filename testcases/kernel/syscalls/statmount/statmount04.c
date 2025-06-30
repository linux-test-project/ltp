// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that statmount() is correctly reading propagation from
 * what mount in current namespace using STATMOUNT_PROPAGATE_FROM.
 *
 * [Algorithm]
 *
 * - create a mount point
 * - propagate a mounted folder inside the mount point
 * - run statmount() on the mount point using STATMOUNT_PROPAGATE_FROM
 * - read results and check propagated_from parameter contains the propagated
 *   folder ID
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"

#define MNTPOINT "mntpoint"
#define DIR_A MNTPOINT "/LTP_DIR_A"
#define DIR_C_SUBFOLDER "/LTP_DIR_A/propagated"
#define DIR_C (MNTPOINT DIR_C_SUBFOLDER)
#define DIR_B MNTPOINT "/LTP_DIR_B"
#define DIR_D MNTPOINT "/LTP_DIR_B/propagated"

static uint64_t peer_group_id;
static uint64_t dird_id;
static struct statmount *st_mount;

static void run(void)
{
	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_PASS(statmount(dird_id, STATMOUNT_PROPAGATE_FROM, st_mount,
		sizeof(struct statmount), 0));

	if (!TST_PASS)
		return;

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_PROPAGATE_FROM);
	TST_EXP_EQ_LI(st_mount->size, sizeof(struct statmount));
	TST_EXP_EQ_LI(st_mount->propagate_from, peer_group_id);
}

static void setup(void)
{
	struct ltp_statx sx;

	/* create DIR_A / DIR_C structure with DIR_C mounted */
	SAFE_MKDIR(DIR_A, 0700);
	SAFE_MOUNT(DIR_A, DIR_A, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", DIR_A, "none", MS_SHARED, NULL);

	SAFE_MKDIR(DIR_C, 0700);
	SAFE_MOUNT(DIR_C, DIR_C, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", DIR_C, "none", MS_SHARED, NULL);

	/* DIR_A mounts into DIR_B. DIR_D is propagated */
	SAFE_MKDIR(DIR_B, 0700);
	SAFE_MOUNT(DIR_A, DIR_B, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", DIR_B, "none", MS_SLAVE, NULL);

	SAFE_STATX(AT_FDCWD, DIR_D, 0, STATX_MNT_ID_UNIQUE, &sx);
	dird_id = sx.data.stx_mnt_id;

	peer_group_id = read_peer_group(DIR_A);
}

static void cleanup(void)
{
	if (tst_is_mounted(DIR_C))
		SAFE_UMOUNT(DIR_C);

	if (tst_is_mounted(DIR_B))
		SAFE_UMOUNT(DIR_B);

	if (tst_is_mounted(DIR_A))
		SAFE_UMOUNT(DIR_A);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{}
	}
};
