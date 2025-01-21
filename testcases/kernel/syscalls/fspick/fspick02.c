// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fspick() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT		"mntpoint"

static struct tcase {
	char *name;
	int dirfd;
	const char *pathname;
	unsigned int flags;
	int exp_errno;
} tcases[] = {
	{"invalid-fd", -1, MNTPOINT, FSPICK_NO_AUTOMOUNT | FSPICK_CLOEXEC, EBADF},
	{"invalid-path", AT_FDCWD, "invalid", FSPICK_NO_AUTOMOUNT | FSPICK_CLOEXEC, ENOENT},
	{"invalid-flags", AT_FDCWD, MNTPOINT, 0x10, EINVAL},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fspick(tc->dirfd, tc->pathname, tc->flags));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_res(TFAIL, "%s: fspick() succeeded unexpectedly (index: %d)",
			tc->name, n);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: fspick() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: fspick() failed as expected", tc->name);
}

static struct tst_test test = {
	.timeout = 9,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = fsopen_supported_by_kernel,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.skip_filesystems = (const char *const []){"fuse", NULL},
};
