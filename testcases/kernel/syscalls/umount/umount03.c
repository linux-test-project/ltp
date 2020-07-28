// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 * Verify that umount(2) returns -1 and sets errno to  EPERM if the user
 * is not the super-user.
 */
/* Patch to use root file system in lkl as default LKL memory is set to 
 * 32M
 */
#include <errno.h>
#include <pwd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag;

static void verify_umount(void)
{
	TEST(umount(MNTPOINT));

	if (TST_RET != -1) {
		tst_res(TFAIL, "umount() succeeds unexpectedly");
		return;
	}

	if (TST_ERR != EPERM) {
		tst_res(TFAIL | TTERRNO, "umount() should fail with EPERM");
		return;
	}

	tst_res(TPASS | TTERRNO, "umount() fails as expected");
}

static void setup(void)
{
	struct passwd *pw;
	
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, 0775);

	SAFE_MOUNT("/dev/vda", MNTPOINT, "ext4", 0, NULL);
	mount_flag = 1;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pw->pw_uid);
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_res(TWARN | TERRNO, "seteuid(0) Failed");

	if (mount_flag)
		tst_umount(MNTPOINT);
	SAFE_RMDIR(MNTPOINT);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 0,
	.format_device = 0,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_umount,
};
