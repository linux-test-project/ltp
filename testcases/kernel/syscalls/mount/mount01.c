// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *               Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Basic test that checks mount() syscall works on multiple filesystems.
 */

#include "tst_test.h"
#include <sys/mount.h>

#define MNTPOINT "mntpoint"

static void cleanup(void)
{
	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static void run(void)
{
	TST_EXP_PASS(mount(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL));

	if (tst_is_mounted(MNTPOINT)) {
		tst_res(TPASS, "folder has been mounted");
		SAFE_UMOUNT(MNTPOINT);
	} else {
		tst_res(TFAIL, "folder has not been mounted");
	}
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.format_device = 1,
	.all_filesystems = 1,
	.mntpoint = MNTPOINT,
	.skip_filesystems = (const char *const []){
		"ntfs",
		NULL
	},
};
