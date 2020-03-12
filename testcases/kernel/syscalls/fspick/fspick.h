// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef FSPICK_H__
#define FSPICK_H__

#define MNTPOINT	"mntpoint"

static int ismounted;

static void cleanup(void)
{
	if (ismounted)
		SAFE_UMOUNT(MNTPOINT);
}

static void setup(void)
{
	int fd, fsmfd;

	fsopen_supported_by_kernel();

	TEST(fd = fsopen(tst_device->fs_type, 0));
	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	TEST(fsmfd = fsmount(fd, 0, 0));
	SAFE_CLOSE(fd);

	if (fsmfd == -1)
		tst_brk(TBROK | TERRNO, "fsmount() failed");

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "move_mount() failed");

	if (!tst_is_mounted_at_tmpdir(MNTPOINT))
		tst_brk(TBROK | TERRNO, "device not mounted");

	ismounted = 1;
}

#endif /* FSPICK_H__ */
