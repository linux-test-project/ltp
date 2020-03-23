// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic move_mount() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

#define TCASE_ENTRY(_flags)	{.name = "Flag " #_flags, .flags = _flags}

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	TCASE_ENTRY(MOVE_MOUNT_F_SYMLINKS),
	TCASE_ENTRY(MOVE_MOUNT_F_AUTOMOUNTS),
	TCASE_ENTRY(MOVE_MOUNT_F_EMPTY_PATH),
	TCASE_ENTRY(MOVE_MOUNT_T_SYMLINKS),
	TCASE_ENTRY(MOVE_MOUNT_T_AUTOMOUNTS),
	TCASE_ENTRY(MOVE_MOUNT_T_EMPTY_PATH),
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fsmfd, fd;

	TEST(fd = fsopen(tst_device->fs_type, 0));
	if (fd == -1) {
		tst_res(TFAIL | TTERRNO, "fsopen() failed");
		return;
	}

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_STRING) failed");
		return;
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_CMD_CREATE) failed");
		return;
	}

	TEST(fsmfd = fsmount(fd, 0, 0));
	SAFE_CLOSE(fd);

	if (fsmfd == -1) {
		tst_res(TFAIL | TTERRNO, "fsmount() failed");
		return;
	}

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			tc->flags | MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "move_mount() failed");
		return;
	}

	if (tst_is_mounted_at_tmpdir(MNTPOINT)) {
		SAFE_UMOUNT(MNTPOINT);
		tst_res(TPASS, "%s: move_mount() passed", tc->name);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = fsopen_supported_by_kernel,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
