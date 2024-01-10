// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *               Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that mount(2) returns -1 and sets errno to EPERM if the user
 * is not root.
 */

#include "tst_test.h"
#include <pwd.h>
#include <sys/mount.h>

#define MNTPOINT "mntpoint"

static void cleanup(void)
{
	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static void run(void)
{
	struct passwd *ltpuser;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);

	TST_EXP_FAIL(
		mount(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL),
		EPERM,
		"mount() failed expectedly for normal user"
	);

	if (tst_is_mounted(MNTPOINT))
		SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = run,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
};
