// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies STATMOUNT_MNT_ROOT and STATMOUNT_MNT_POINT functionalities
 * of statmount(). In particular, STATMOUNT_MNT_ROOT will give the mount root
 * (i.e. mount --bind /mnt /bla -> /mnt) and STATMOUNT_MNT_POINT will
 * give the mount point (i.e. mount --bind /mnt /bla -> /bla).
 *
 * [Algorithm]
 *
 * - create a mount point
 * - mount a folder inside the mount point
 * - run statmount() on the mounted folder using STATMOUNT_MNT_ROOT
 * - read results and check if contain the mount root path
 * - run statmount() on the mounted folder using STATMOUNT_MNT_POINT
 * - read results and check if contain the mount point path
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/sched.h"
#include "tst_tmpdir.h"

#define MNTPOINT "mntpoint"
#define DIRA MNTPOINT "/LTP_DIR_A"
#define DIRB MNTPOINT "/LTP_DIR_B"
#define SM_SIZE (1 << 10)

static uint64_t root_id;
static struct statmount *st_mount;
static char *mnt_root;
static char *mnt_point;

static void test_mount_root(void)
{
	tst_res(TINFO, "Testing STATMOUNT_MNT_ROOT");

	char *last_root;

	memset(st_mount, 0, SM_SIZE);

	TST_EXP_PASS(statmount(root_id, STATMOUNT_MNT_ROOT, st_mount,
		SM_SIZE, 0));

	if (!TST_PASS)
		return;

	last_root = strrchr(mnt_root, '/');

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_MNT_ROOT);
	TST_EXP_EQ_STR(st_mount->str + st_mount->mnt_root, last_root);
}

static void test_mount_point(void)
{
	tst_res(TINFO, "Testing STATMOUNT_MNT_POINT");

	memset(st_mount, 0, SM_SIZE);

	TST_EXP_POSITIVE(statmount(root_id, STATMOUNT_MNT_POINT, st_mount,
		SM_SIZE, 0));

	if (!TST_PASS)
		return;

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_MNT_POINT);
	TST_EXP_EQ_STR(st_mount->str + st_mount->mnt_point, mnt_point);
}

static void run(void)
{
	test_mount_root();
	test_mount_point();
}

static void setup(void)
{
	struct ltp_statx sx;

	mnt_root = tst_tmpdir_genpath(DIRA);
	mnt_point = tst_tmpdir_genpath(DIRB);

	SAFE_MKDIR(mnt_root, 0700);
	SAFE_MKDIR(mnt_point, 0700);
	SAFE_MOUNT(mnt_root, mnt_point, "none", MS_BIND, NULL);

	SAFE_STATX(AT_FDCWD, mnt_point, 0, STATX_MNT_ID_UNIQUE, &sx);
	root_id = sx.data.stx_mnt_id;
}

static void cleanup(void)
{
	if (tst_is_mounted(DIRB))
		SAFE_UMOUNT(DIRB);

	if (tst_is_mounted(DIRA))
		SAFE_UMOUNT(DIRA);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []) {
		"fuse",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = SM_SIZE},
		{}
	}
};
