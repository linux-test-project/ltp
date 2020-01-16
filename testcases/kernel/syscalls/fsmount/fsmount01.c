// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Red Hat, Inc.  All rights reserved.
 * Author: Zorro Lang <zlang@redhat.com>
 *
 * Use new mount API from v5.2 (fsopen(), fsconfig(), fsmount(), move_mount())
 * to mount a filesystem without any specified mount options.
 */

#include <sys/mount.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/fsmount.h"
#include "tst_safe_stdio.h"

#define LINELENGTH 256
#define MNTPOINT "newmount_point"
static int sfd, mfd, is_mounted;

static int ismount(char *mntpoint)
{
	int ret = 0;
	FILE *file;
	char line[LINELENGTH];

	file = SAFE_FOPEN("/proc/mounts", "r");

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, mntpoint) != NULL) {
			ret = 1;
			break;
		}
	}
	SAFE_FCLOSE(file);
	return ret;
}

static void cleanup(void)
{
	if (is_mounted)
		SAFE_UMOUNT(MNTPOINT);
}

static void test_fsmount(void)
{
	TEST(fsopen(tst_device->fs_type, FSOPEN_CLOEXEC));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "fsopen() on %s failed", tst_device->fs_type);
	sfd = TST_RET;
	tst_res(TPASS, "fsopen() on %s", tst_device->fs_type);

	TEST(fsconfig(sfd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO,
			"fsconfig() failed to set source to %s", tst_device->dev);
	tst_res(TPASS, "fsconfig() set source to %s", tst_device->dev);


	TEST(fsconfig(sfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "fsconfig() created superblock");
	tst_res(TPASS, "fsconfig() created superblock");

	TEST(fsmount(sfd, FSMOUNT_CLOEXEC, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "fsmount() failed to create a mount object");
	mfd = TST_RET;
	tst_res(TPASS, "fsmount() created a mount object");
	SAFE_CLOSE(sfd);

	TEST(move_mount(mfd, "", AT_FDCWD, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "move_mount() failed to attach to the mount point");
	is_mounted = 1;
	tst_res(TPASS, "move_mount() attached to the mount point");
	SAFE_CLOSE(mfd);

	if (ismount(MNTPOINT)) {
		tst_res(TPASS, "device mounted");
		SAFE_UMOUNT(MNTPOINT);
		is_mounted = 0;
	} else {
		tst_res(TFAIL, "device not mounted");
	}
}

static struct tst_test test = {
	.test_all = test_fsmount,
	.cleanup = cleanup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
