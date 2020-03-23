// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fsconfig() test which tries to configure and mount the filesystem as
 * well.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static int fd;

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	int fsmfd;

	TEST(fd = fsopen(tst_device->fs_type, 0));
	if (fd == -1)
		tst_brk(TBROK | TTERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_FLAG, "rw", NULL, 0));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_FLAG) failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_STRING) failed");

	TEST(fsconfig(fd, FSCONFIG_SET_PATH, "sync", tst_device->dev, 0));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP)
			tst_res(TCONF, "fsconfig(FSCONFIG_SET_PATH) not supported");
		else
			tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_PATH) failed");
	}

	TEST(fsconfig(fd, FSCONFIG_SET_PATH_EMPTY, "sync", tst_device->dev, 0));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP)
			tst_res(TCONF, "fsconfig(FSCONFIG_SET_PATH_EMPTY) not supported");
		else
			tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_PATH_EMPTY) failed");
	}

	TEST(fsconfig(fd, FSCONFIG_SET_FD, "sync", NULL, 0));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP)
			tst_res(TCONF, "fsconfig(FSCONFIG_SET_FD) not supported");
		else
			tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_FD) failed");
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "fsconfig(FSCONFIG_CMD_CREATE) failed");

	TEST(fsmfd = fsmount(fd, 0, 0));
	if (fsmfd == -1)
		tst_brk(TBROK | TTERRNO, "fsmount() failed");

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "move_mount() failed");

	if (tst_is_mounted_at_tmpdir(MNTPOINT)) {
		SAFE_UMOUNT(MNTPOINT);
		tst_res(TPASS, "fsconfig() passed");
	}

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = fsopen_supported_by_kernel,
	.cleanup = cleanup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
