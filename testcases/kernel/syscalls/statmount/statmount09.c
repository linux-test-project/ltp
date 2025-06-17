// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that statmount() is correctly reading the mount ID for
 * the current namespace.
 *
 * [Algorithm]
 *
 * - create a mount point
 * - run statmount() on the mount point using STATMOUNT_MNT_NS_ID
 * - read results and check if contain the correct mount ID for the current
 *   namespace
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"
#include "lapi/ioctl_ns.h"

#define MNTPOINT "mntpoint"
#define SM_SIZE (1 << 10)

static uint64_t root_id;
static u_int64_t mnt_ns_id;
static struct statmount *st_mount;

static void run(void)
{
	memset(st_mount, 0, SM_SIZE);

	TST_EXP_PASS(statmount(root_id,	STATMOUNT_MNT_NS_ID, st_mount,
		SM_SIZE, 0));

	if (!TST_PASS)
		return;

	TST_EXP_EQ_LI(st_mount->mask, STATMOUNT_MNT_NS_ID);
	TST_EXP_EQ_LI(st_mount->mnt_ns_id, mnt_ns_id);
}

static void setup(void)
{
	struct ltp_statx sx;
	int fd;

	SAFE_STATX(AT_FDCWD, MNTPOINT, 0, STATX_MNT_ID_UNIQUE, &sx);
	root_id = sx.data.stx_mnt_id;

	fd = SAFE_OPEN("/proc/self/ns/mnt", O_RDONLY);
	SAFE_IOCTL(fd, NS_GET_MNTNS_ID, &mnt_ns_id);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.min_kver = "6.11",
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

