// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fspick() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT		"mntpoint"
#define TCASE_ENTRY(_flags)	{.name = "Flag " #_flags, .flags = _flags}

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	TCASE_ENTRY(FSPICK_CLOEXEC),
	TCASE_ENTRY(FSPICK_SYMLINK_NOFOLLOW),
	TCASE_ENTRY(FSPICK_NO_AUTOMOUNT),
	TCASE_ENTRY(FSPICK_EMPTY_PATH),
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fspick_fd;

	TEST(fspick_fd = fspick(AT_FDCWD, MNTPOINT, tc->flags));
	if (fspick_fd == -1) {
		tst_res(TFAIL | TTERRNO, "fspick() failed");
		return;
	}

	TEST(fsconfig(fspick_fd, FSCONFIG_SET_STRING, "sync", "false", 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_STRING) failed");
		goto out;
	}

	TEST(fsconfig(fspick_fd, FSCONFIG_SET_FLAG, "ro", NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_SET_FLAG) failed");
		goto out;
	}

	TEST(fsconfig(fspick_fd, FSCONFIG_CMD_RECONFIGURE, NULL, NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fsconfig(FSCONFIG_CMD_RECONFIGURE) failed");
		goto out;
	}

	tst_res(TPASS, "%s: fspick() passed", tc->name);

out:
	SAFE_CLOSE(fspick_fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = fsopen_supported_by_kernel,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
