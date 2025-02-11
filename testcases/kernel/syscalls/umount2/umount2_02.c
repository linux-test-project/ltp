// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2022 FUJITSU LIMITED. All rights reserved
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*\
 * Test for feature MNT_EXPIRE of umount2():
 *
 * - EINVAL when flag is specified with either MNT_FORCE or MNT_DETACH
 * - EAGAIN when initial call to umount2(2) with MNT_EXPIRE
 * - EAGAIN when umount2(2) with MNT_EXPIRE after access(2)
 * - succeed when second call to umount2(2) with MNT_EXPIRE
 *
 * Test for feature UMOUNT_NOFOLLOW of umount2():
 *
 * - EINVAL when target is a symbolic link
 * - succeed when target is a mount point
 */

#include "lapi/mount.h"
#include "tst_test.h"

#define MNTPOINT        "mntpoint"
#define SYMLINK	"symlink"

#define FLAG_DESC(x, y) .flag = x, .exp_errno = 0, \
	.desc = "umount2("y") with "#x" expected success"

#define FLAG_EXP_ERRNO_DESC(x, y, z) .flag = x, .exp_errno = y, \
	.desc = "umount2("z") with "#x" expected "#y

static int mount_flag;

static struct tcase {
	int flag;
	int exp_errno;
	const char *desc;
	const char *mntpoint;
	int do_access;
} tcases[] = {
	{FLAG_EXP_ERRNO_DESC(MNT_EXPIRE | MNT_FORCE, EINVAL, ""), MNTPOINT, 0},
	{FLAG_EXP_ERRNO_DESC(MNT_EXPIRE | MNT_DETACH, EINVAL, ""), MNTPOINT, 0},
	{FLAG_EXP_ERRNO_DESC(MNT_EXPIRE, EAGAIN, "initial call"), MNTPOINT, 0},
	{FLAG_EXP_ERRNO_DESC(MNT_EXPIRE, EAGAIN, "after access"), MNTPOINT, 1},
	{FLAG_DESC(MNT_EXPIRE, "second call"), MNTPOINT, 0},
	{FLAG_EXP_ERRNO_DESC(UMOUNT_NOFOLLOW, EINVAL, "symlink"), SYMLINK, 0},
	{FLAG_DESC(UMOUNT_NOFOLLOW, "mntpoint"), MNTPOINT, 0},
};

static int umount2_retry(const char *target, int flags)
{
	int i, ret;

	for (i = 0; i < 50; i++) {
		ret = umount2(target, flags);
		if (ret == 0 || errno != EBUSY)
			return ret;

		tst_res(TINFO, "umount('%s', %i) failed with EBUSY, try %2i...",
			target, flags, i);

		usleep(100000);
	}

	tst_res(TWARN, "Failed to umount('%s', %i) after 50 retries",
		target, flags);

	errno = EBUSY;
	return -1;
}

static void test_umount2(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (!mount_flag) {
		SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL);
		mount_flag = 1;
	}

	tst_res(TINFO, "Testing %s", tc->desc);

	if (tc->do_access)
		SAFE_ACCESS(MNTPOINT, F_OK);

	if (tc->exp_errno)
		TST_EXP_FAIL(umount2_retry(tc->mntpoint, tc->flag), tc->exp_errno,
			"umount2_retry(%s, %d)", tc->mntpoint, tc->flag);
	else
		TST_EXP_PASS(umount2_retry(tc->mntpoint, tc->flag),
			"umount2_retry(%s, %d)", tc->mntpoint, tc->flag);

	if (!!tc->exp_errno ^ !!TST_PASS)
		mount_flag = 0;
}

static void setup(void)
{
	SAFE_SYMLINK(MNTPOINT, SYMLINK);
}

static void cleanup(void)
{
	if (mount_flag)
		SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.cleanup = cleanup,
	.setup = setup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.test = test_umount2,
};
