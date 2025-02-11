// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2002-2023
 * Author: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 */

/*\
 * Verify that umount(2) returns -1 and sets errno to EPERM if the user
 * is not the super-user.
 */

#include <pwd.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag;

static void verify_umount(void)
{
	TST_EXP_FAIL(umount(MNTPOINT), EPERM);
}

static void setup(void)
{
	struct passwd *pw;

	SAFE_MKDIR(MNTPOINT, 0775);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL);
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
}

static struct tst_test test = {
	.needs_root = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_umount,
};
