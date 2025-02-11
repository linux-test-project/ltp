// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * This is not only a functional test but also a error test for Q_XQUOTARM.
 *
 * It is a regresstion test for kernel commit 3dd4d40b4208
 * ("xfs: Sanity check flags of Q_XQUOTARM call").
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/quota.h>
#include <sys/statvfs.h>
#include "tst_test.h"
#include "quotactl_syscall_var.h"

#ifdef HAVE_XFS_XQM_H
# include <xfs/xqm.h>

/* Include a valid quota type to avoid other EINVAL error */
static unsigned int invalid_type = XFS_GROUP_QUOTA << 1 | XFS_USER_QUOTA;
static unsigned int valid_type = XFS_USER_QUOTA;
static int mount_flag;

static void verify_quota(void)
{
	struct statfs before, after;

	SAFE_STATFS(MNTPOINT, &before);
	TST_EXP_PASS(do_quotactl(fd, QCMD(Q_XQUOTARM, USRQUOTA), tst_device->dev, 0,
			(void *)&valid_type), "do_quotactl(Q_XQUOTARM,valid_type)");
	SAFE_STATFS(MNTPOINT, &after);
	if (before.f_bavail <= after.f_bavail)
		tst_res(TPASS, "Q_XQUOTARM to free space, delta(%lu)", after.f_bavail - before.f_bavail);
	else
		tst_res(TFAIL, "Q_XQUOTARM to free space, delta(-%lu)", before.f_bavail - after.f_bavail);

	TST_EXP_FAIL(do_quotactl(fd, QCMD(Q_XQUOTARM, USRQUOTA), tst_device->dev, 0,
			(void *)&invalid_type), EINVAL, "do_quotactl(Q_XQUOTARM, invalid_type)");
}

static void setup(void)
{
	quotactl_info();

	/*
	 * Ensure superblock has quota data, but not running. In here, we must unmount
	 * completely and mount again with '-o no quota' because 'mount -o remount, noquota'
	 * isn't sufficient to disable accounting feature.
	 */
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, "usrquota");
	mount_flag = 1;
	SAFE_UMOUNT(MNTPOINT);
	mount_flag = 0;
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, "noquota");
	mount_flag = 1;

	fd = SAFE_OPEN(MNTPOINT, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
	if (mount_flag && tst_umount(MNTPOINT))
		tst_res(TWARN | TERRNO, "umount(%s)", MNTPOINT);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_XFS_QUOTA",
		NULL
	},
	.test_all = verify_quota,
	.format_device = 1,
	.filesystems = (struct tst_fs []) {
		{.type = "xfs"},
		{}
	},
	.mntpoint = MNTPOINT,
	.test_variants = QUOTACTL_SYSCALL_VARIANTS,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3dd4d40b4208"},
		{}
	}
};
#else
	TST_TEST_TCONF("System doesn't have <xfs/xqm.h>");
#endif
