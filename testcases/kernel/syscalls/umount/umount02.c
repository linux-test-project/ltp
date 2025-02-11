// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2002-2023
 * Author: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 */

/*\
 * Check for basic errors returned by umount(2) system call.
 *
 * Verify that umount(2) returns -1 and sets errno to
 *
 * 1. EBUSY if it cannot be umounted, because dir is still busy.
 * 2. EFAULT if specialfile or device file points to invalid address space.
 * 3. ENOENT if pathname was empty or has a nonexistent component.
 * 4. EINVAL if specialfile or device is invalid or not a mount point.
 * 5. ENAMETOOLONG if pathname was longer than MAXPATHLEN.
 */

#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static char long_path[PATH_MAX + 2];
static int mount_flag;
static int fd;

static struct tcase {
	const char *err_desc;
	const char *mntpoint;
	int exp_errno;
} tcases[] = {
	{"Already mounted/busy", MNTPOINT, EBUSY},
	{"Invalid address", NULL, EFAULT},
	{"Directory not found", "nonexistent", ENOENT},
	{"Invalid  device", "./", EINVAL},
	{"Pathname too long", long_path, ENAMETOOLONG}
};

static void verify_umount(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(umount(tc->mntpoint), tc->exp_errno);
}

static void setup(void)
{
	memset(long_path, 'a', PATH_MAX + 1);

	SAFE_MKDIR(MNTPOINT, 0775);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL);
	mount_flag = 1;

	fd = SAFE_CREAT(MNTPOINT "/file", 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_umount,
};
