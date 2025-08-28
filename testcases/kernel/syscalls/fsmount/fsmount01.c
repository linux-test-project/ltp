// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Red Hat, Inc.  All rights reserved.
 * Author: Zorro Lang <zlang@redhat.com>
 *
 * Basic fsmount() test.
 */

#include "tst_test.h"
#include "lapi/fsmount.h"
#include "tst_safe_stdio.h"

#define MNTPOINT	"mntpoint"

#define TCASE_ENTRY(_flags, _attrs)	{.name = "Flag " #_flags ", Attr " #_attrs, .flags = _flags, .attrs = _attrs}

static struct tcase {
	char *name;
	unsigned int flags;
	unsigned int attrs;
} tcases[] = {
	TCASE_ENTRY(0, MOUNT_ATTR_RDONLY),
	TCASE_ENTRY(0, MOUNT_ATTR_NOSUID),
	TCASE_ENTRY(0, MOUNT_ATTR_NODEV),
	TCASE_ENTRY(0, MOUNT_ATTR_NOEXEC),
	TCASE_ENTRY(0, MOUNT_ATTR_RELATIME),
	TCASE_ENTRY(0, MOUNT_ATTR_NOATIME),
	TCASE_ENTRY(0, MOUNT_ATTR_STRICTATIME),
	TCASE_ENTRY(0, MOUNT_ATTR_NODIRATIME),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_RDONLY),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_NOSUID),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_NODEV),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_NOEXEC),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_RELATIME),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_NOATIME),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_STRICTATIME),
	TCASE_ENTRY(FSMOUNT_CLOEXEC, MOUNT_ATTR_NODIRATIME),
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int sfd, mfd;

	TEST(sfd = fsopen(tst_device->fs_type, FSOPEN_CLOEXEC));
	if (sfd == -1) {
		tst_res(TFAIL | TTERRNO, "fsopen() on %s failed",
			tst_device->fs_type);
		return;
	}

	TEST(fsconfig(sfd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(sfd);
		tst_res(TFAIL | TTERRNO,
			"fsconfig(FSCONFIG_SET_STRING) failed to set source to %s", tst_device->dev);
		return;
	}

	TEST(fsconfig(sfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(sfd);
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_CMD_CREATE) failed");
		return;
	}

	TEST(mfd = fsmount(sfd, tc->flags, tc->attrs));
	SAFE_CLOSE(sfd);

	if (mfd == -1) {
		tst_res(TFAIL | TTERRNO,
			"fsmount() failed to create a mount object");
		return;
	}

	TEST(move_mount(mfd, "", AT_FDCWD, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(mfd);

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO,
			"move_mount() failed to attach to the mount point");
		return;
	}

	switch (tc->attrs) {
	case MOUNT_ATTR_RDONLY:
		TST_EXP_VAL(tst_is_mounted_ro(MNTPOINT), 1);
		break;
	case MOUNT_ATTR_NOSUID:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "nosuid"), 1);
		break;
	case MOUNT_ATTR_NODEV:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "nodev"), 1);
		break;
	case MOUNT_ATTR_NOEXEC:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "noexec"), 1);
		break;
	case MOUNT_ATTR_RELATIME:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "relatime"), 1);
		break;
	case MOUNT_ATTR_NOATIME:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "noatime"), 1);
		break;
	case MOUNT_ATTR_NODIRATIME:
		TST_EXP_VAL(tst_mount_has_opt(MNTPOINT, "nodiratime"), 1);
		break;
	default:
		break;
	}

	if (tst_is_mounted_at_tmpdir(MNTPOINT)) {
		SAFE_UMOUNT(MNTPOINT);
		tst_res(TPASS, "%s: fsmount() passed", tc->name);
	}
}

static struct tst_test test = {
	.timeout = 10,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = fsopen_supported_by_kernel,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){"fuse", NULL},
};
