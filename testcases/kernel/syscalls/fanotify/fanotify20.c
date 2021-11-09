// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Google. All Rights Reserved.
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

static int fanotify_fd;

static struct test_case_t {
	char *name;
	unsigned int init_flags;
	int want_err;
	int want_errno;
} test_cases[] = {
	{
		"fail on FAN_REPORT_PIDFD | FAN_REPORT_TID",
		FAN_REPORT_PIDFD | FAN_REPORT_TID,
		1,
		EINVAL,
	},
	{
		"pass on FAN_REPORT_PIDFD | FAN_REPORT_FID | FAN_REPORT_DFID_NAME",
		FAN_REPORT_PIDFD | FAN_REPORT_FID | FAN_REPORT_DFID_NAME,
		0,
		0,
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

static void do_test(unsigned int num)
{
	struct test_case_t *tc = &test_cases[num];

	tst_res(TINFO, "Test #%d: %s", num, tc->name);

	fanotify_fd = fanotify_init(tc->init_flags, O_RDONLY);
	if (fanotify_fd < 0) {
		if (!tc->want_err) {
			tst_res(TFAIL,
				"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
				"failed with error -%d but wanted success",
				fanotify_fd, tc->init_flags, errno);
			return;
		}

		if (errno != tc->want_errno) {
			tst_res(TFAIL,
				"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
				"failed with an unexpected error code -%d but "
				"wanted -%d",
				fanotify_fd, tc->init_flags,
				errno, tc->want_errno);
			return;
		}

		tst_res(TPASS,
			"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
			"failed with error -%d as expected",
			fanotify_fd, tc->init_flags, errno);
		return;
	}

	/*
	 * Catch test cases that had expected to receive an error upon calling
	 * fanotify_init() but had unexpectedly resulted in a success.
	 */
	if (tc->want_err) {
		tst_res(TFAIL,
			"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
			"unexpectedly returned successfully, wanted error -%d",
			fanotify_fd, tc->init_flags, tc->want_errno);
		return;
	}

	tst_res(TPASS,
		"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
		"successfully initialized notification group",
		fanotify_fd, tc->init_flags);

	SAFE_CLOSE(fanotify_fd);
}

static void do_cleanup(void)
{
	if (fanotify_fd >= 0)
		SAFE_CLOSE(fanotify_fd);
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
