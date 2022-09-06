// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Google. All Rights Reserved.
 * Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
 *
 * Started by Matthew Bobrowski <repnop@google.com>
 */

/*\
 * [Description]
 *
 * This source file contains a test case which ensures that the fanotify API
 * returns an expected error code when provided an invalid initialization flag
 * alongside FAN_REPORT_PIDFD. Additionally, it checks that the operability with
 * existing FAN_REPORT_* flags is maintained and functioning as intended.
 *
 * NOTE: FAN_REPORT_PIDFD support was added in v5.15-rc1 in af579beb666a
 * ("fanotify: add pidfd support to the fanotify API").
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include <errno.h>

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define MOUNT_PATH	"fs_mnt"
#define FLAGS_DESC(x) .flags = x, .desc = #x

static int fd;

static struct test_case_t {
	unsigned int flags;
	char *desc;
	int exp_errno;
} test_cases[] = {
	{
		FLAGS_DESC(FAN_REPORT_PIDFD | FAN_REPORT_TID),
		.exp_errno = EINVAL,
	},
	{
		FLAGS_DESC(FAN_REPORT_PIDFD | FAN_REPORT_FID | FAN_REPORT_DFID_NAME),
	},
};

static void do_setup(void)
{
	/*
	 * An explicit check for FAN_REPORT_PIDFD is performed early on in the
	 * test initialization as it's a prerequisite for all test cases.
	 */
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_BY_KERNEL(FAN_REPORT_PIDFD);
}

static void do_test(unsigned int i)
{
	struct test_case_t *tc = &test_cases[i];

	tst_res(TINFO, "Test %s on %s", tc->exp_errno ? "fail" : "pass",
		tc->desc);

	TST_EXP_FD_OR_FAIL(fd = fanotify_init(tc->flags, O_RDONLY),
			   tc->exp_errno);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void do_cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = do_setup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.cleanup = do_cleanup,
	.all_filesystems = 1,
	.needs_root = 1,
	.mntpoint = MOUNT_PATH,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif /* HAVE_SYS_FANOTIFY_H */
