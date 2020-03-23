// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fsmount() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

int fd = -1, invalid_fd = -1;

#define MNTPOINT	"mntpoint"

static struct tcase {
	char *name;
	int *fd;
	unsigned int flags;
	unsigned int mount_attrs;
	int exp_errno;
} tcases[] = {
	{"invalid-fd", &invalid_fd, FSMOUNT_CLOEXEC, 0, EBADF},
	{"invalid-flags", &fd, 0x02, 0, EINVAL},
	{"invalid-attrs", &fd, FSMOUNT_CLOEXEC, 0x100, EINVAL},
};

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	fsopen_supported_by_kernel();

	TEST(fd = fsopen(tst_device->fs_type, 0));
	if (fd == -1)
		tst_brk(TBROK | TTERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "fsconfig(FSCONFIG_SET_STRING) failed");

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "fsconfig(FSCONFIG_CMD_CREATE) failed");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fsmount(*tc->fd, tc->flags, tc->mount_attrs));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_res(TFAIL, "%s: fsmount() succeeded unexpectedly (index: %d)",
			tc->name, n);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: fsmount() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: fsmount() failed as expected", tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
