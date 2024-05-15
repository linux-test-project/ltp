// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that statmount() is correctly reading mount information
 * (mount id, parent mount id, mount attributes etc.) using STATMOUNT_MNT_BASIC.
 *
 * [Algorithm]
 *
 * - create a mount point
 * - create a new parent folder inside the mount point and obtain its mount info
 * - create the new "/" mount folder and obtain its mount info
 * - run statmount() on the mount point using STATMOUNT_MNT_BASIC
 * - read results and check if mount info are correct
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"

#define DIR_A "LTP_DIR_A"
#define DIR_B "LTP_DIR_B"

static uint64_t mnt_id;
static uint64_t mnt_id_old;
static uint64_t parent_id;
static uint64_t parent_id_old;
static uint64_t mnt_peer_group;
static uint64_t mnt_master;
static struct statmount *st_mount;

static void read_mnt_id(
	const char *path,
	uint64_t *mnt_id,
	uint64_t *mnt_id_unique)
{
	struct ltp_statx sx;

	if (mnt_id) {
		sx.data.stx_mask = STATX_MNT_ID;

		SAFE_STATX(AT_FDCWD, path, 0, STATX_MNT_ID, &sx);
		*mnt_id = sx.data.stx_mnt_id;
	}

	if (mnt_id_unique) {
		sx.data.stx_mask = STATX_MNT_ID_UNIQUE;

		SAFE_STATX(AT_FDCWD, path, 0, STATX_MNT_ID_UNIQUE, &sx);
		*mnt_id_unique = sx.data.stx_mnt_id;
	}
}

static struct tcase {
	uint64_t prop_type;
	char *msg;
} tcases[] = {
	{ MS_PRIVATE, TST_TO_STR_(MS_PRIVATE) },
	{ MS_SHARED, TST_TO_STR_(MS_SHARED) },
	{ MS_SLAVE, TST_TO_STR_(MS_SLAVE) },
	{ MS_UNBINDABLE, TST_TO_STR_(MS_UNBINDABLE) },
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	tst_res(TINFO,  "Testing statmount() on %s", tc->msg);

	SAFE_MOUNT(DIR_B, DIR_A, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", DIR_A, "none", tc->prop_type, NULL);
	read_mnt_id(DIR_A, &mnt_id_old, &mnt_id);

	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_PASS(statmount(mnt_id, STATMOUNT_MNT_BASIC, st_mount,
		sizeof(struct statmount), 0));

	SAFE_UMOUNT(DIR_A);

	if (!TST_PASS)
		return;

	mnt_peer_group = tc->prop_type == MS_SHARED ? read_peer_group(DIR_A) : 0;
	mnt_master = tc->prop_type == MS_SLAVE ? read_peer_group(DIR_A) : 0;

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_MNT_BASIC);
	TST_EXP_EQ_LI(st_mount->size, sizeof(struct statmount));
	TST_EXP_EQ_LI(st_mount->mnt_id, mnt_id);
	TST_EXP_EQ_LI(st_mount->mnt_id_old, mnt_id_old);
	TST_EXP_EQ_LI(st_mount->mnt_parent_id, parent_id);
	TST_EXP_EQ_LI(st_mount->mnt_parent_id_old, parent_id_old);
	TST_EXP_EQ_LU(st_mount->mnt_propagation & tc->prop_type, tc->prop_type);
	TST_EXP_EQ_LI(st_mount->mnt_master, mnt_master);
	TST_EXP_EQ_LI(st_mount->mnt_peer_group, mnt_peer_group);
}

static void setup(void)
{
	SAFE_MKDIR(DIR_A, 0700);
	SAFE_MKDIR(DIR_B, 0700);

	SAFE_UNSHARE(CLONE_NEWNS);
	SAFE_MOUNT("none", "/", "none", MS_REC | MS_PRIVATE, NULL);

	SAFE_MOUNT(DIR_B, DIR_B, "none", MS_BIND, NULL);
	SAFE_MOUNT("none", DIR_B, "none", MS_SHARED, NULL);

	read_mnt_id(DIR_A, &parent_id_old, &parent_id);
}

static void cleanup(void)
{
	if (tst_is_mounted(DIR_B))
		SAFE_UMOUNT(DIR_B);

	if (tst_is_mounted(DIR_A))
		SAFE_UMOUNT(DIR_A);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "6.8",
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{}
	}
};
