// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * This is a regression test for the race between getting an existing
 * xattr and setting/removing a large xattr.  This bug leads to that
 * getxattr() fails to get an existing xattr and returns ENOATTR in xfs
 * filesystem.
 *
 * This bug has been fixed in:
 * 5a93790d4e2d ("xfs: remove racy hasattr check from attr ops")
 */

#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "tst_test.h"

#ifdef HAVE_SYS_XATTR_H

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT "/file"
#define TRUSTED_BIG	"trusted.big"
#define TRUSTED_SMALL	"trusted.small"

static volatile int end;
static char big_value[512];
static char small_value[32];

static void sigproc(int sig)
{
	end = sig;
}

static void loop_getxattr(void)
{
	int res;

	while (!end) {
		res = getxattr(TEST_FILE, TRUSTED_SMALL, NULL, 0);
		if (res == -1) {
			if (errno == ENODATA) {
				tst_res(TFAIL, "getxattr() failed to get an "
					"existing attribute");
			} else {
				tst_res(TFAIL | TERRNO,
					"getxattr() failed without ENOATTR");
			}

			exit(0);
		}
	}

	tst_res(TPASS, "getxattr() succeeded to get an existing attribute");
	exit(0);
}

static void verify_getxattr(void)
{
	pid_t pid;
	int n;

	end = 0;

	pid = SAFE_FORK();
	if (!pid)
		loop_getxattr();

	for (n = 0; n < 99; n++) {
		SAFE_SETXATTR(TEST_FILE, TRUSTED_BIG, big_value,
				sizeof(big_value), XATTR_CREATE);
		SAFE_REMOVEXATTR(TEST_FILE, TRUSTED_BIG);
	}

	kill(pid, SIGUSR1);
	tst_reap_children();
}

static void setup(void)
{
	SAFE_SIGNAL(SIGUSR1, sigproc);

	SAFE_TOUCH(TEST_FILE, 0644, NULL);

	memset(big_value, 'a', sizeof(big_value));
	memset(small_value, 'a', sizeof(small_value));

	SAFE_SETXATTR(TEST_FILE, TRUSTED_SMALL, small_value,
			sizeof(small_value), XATTR_CREATE);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.dev_fs_type = "xfs",
	.mntpoint = MNTPOINT,
	.forks_child = 1,
	.test_all = verify_getxattr,
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "5a93790d4e2d"},
		{}
	}
};

#else /* HAVE_SYS_XATTR_H */
	TST_TEST_TCONF("<sys/xattr.h> does not exist.");
#endif
