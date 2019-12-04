// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 * Phase I test for the umount(2) system call.
 * It is intended to provide a limited exposure of the system call.
 */

#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag;

static void verify_umount(void)
{
	if (mount_flag != 1) {
		SAFE_MOUNT(tst_device->dev, MNTPOINT,
			tst_device->fs_type, 0, NULL);
		mount_flag = 1;
	}

	TEST(umount(MNTPOINT));

	if (TST_RET != 0 && TST_ERR == EBUSY) {
		tst_res(TINFO, "umount() Failed with EBUSY "
			"possibly some daemon (gvfsd-trash) "
			"is probing newly mounted dirs");
	}

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "umount() Failed");
		return;
	}

	tst_res(TPASS, "umount() Passed");
	mount_flag = 0;
}

static void setup(void)
{
	SAFE_MKDIR(MNTPOINT, 0775);
}

static void cleanup(void)
{
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
