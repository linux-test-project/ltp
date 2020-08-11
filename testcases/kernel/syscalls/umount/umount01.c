// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 * Phase I test for the umount(2) system call.
 * It is intended to provide a limited exposure of the system call.
 */
/* Patching for lkl to use root file system as default LKL memory
 * is set to 32MB.
 */
#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag = 0;

static void verify_umount(void)
{
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
	rmdir(MNTPOINT);
	SAFE_MKDIR(MNTPOINT, 0775);

	const char* src  = "/dev/vda";
	const char* type = "ext4";
	const unsigned long mntflags = 0;
	const char* opts = "mode=0777";
	int result = mount(src, MNTPOINT, type, mntflags, opts);

	if (result != 0) {
		TST_RET = result;
		tst_res(TFAIL, "umount()- setup Failed");
		return;
	}
}

static void cleanup(void)
{
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
