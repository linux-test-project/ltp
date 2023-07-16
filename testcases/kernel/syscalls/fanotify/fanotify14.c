// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Matthew Bobrowski. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2020-2022
 *
 * Started by Matthew Bobrowski <mbobrowski@mbobrowski.org>
 */

/*\
 * [Description]
 * This test file has been designed to ensure that the fanotify
 * system calls fanotify_init(2) and fanotify_mark(2) return the
 * correct error code to the calling process when an invalid flag or
 * mask value has been specified in conjunction with FAN_REPORT_FID.
 */

/*
 * The ENOTDIR test cases are regression tests for commits:
 *
 *     ceaf69f8eadc fanotify: do not allow setting dirent events in mask of non-dir
 *     8698e3bab4dd fanotify: refine the validation checks on non-dir inode mask
 *
 * The pipes test cases are regression tests for commit:
 *     69562eb0bd3e fanotify: disallow mount/sb marks on kernel internal pseudo fs
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include <errno.h>

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define MNTPOINT "mntpoint"
#define FILE1 MNTPOINT"/file1"

/*
 * List of inode events that are only available when notification group is
 * set to report fid.
 */
#define INODE_EVENTS (FAN_ATTRIB | FAN_CREATE | FAN_DELETE | FAN_MOVE | \
		      FAN_DELETE_SELF | FAN_MOVE_SELF)

#define FLAGS_DESC(flags) {(flags), (#flags)}

static int pipes[2] = {-1, -1};
static int fanotify_fd;
static int fan_report_target_fid_unsupported;
static int ignore_mark_unsupported;

struct test_case_flags_t {
	unsigned long long flags;
	const char *desc;
};

/*
 * Each test case has been designed in a manner whereby the values defined
 * within should result in the interface to return an error to the calling
 * process.
 */
static struct test_case_t {
	struct test_case_flags_t init;
	struct test_case_flags_t mark;
	/* when mask.flags == 0, fanotify_init() is expected to fail */
	struct test_case_flags_t mask;
	int expected_errno;
	int *pfd;
} test_cases[] = {
	/* FAN_REPORT_FID without class FAN_CLASS_NOTIF is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_CONTENT | FAN_REPORT_FID),
		.expected_errno = EINVAL,
	},

	/* FAN_REPORT_FID without class FAN_CLASS_NOTIF is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_PRE_CONTENT | FAN_REPORT_FID),
		.expected_errno = EINVAL,
	},

	/* INODE_EVENTS in mask without class FAN_REPORT_FID are not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(INODE_EVENTS),
		.expected_errno = EINVAL,
	},

	/* INODE_EVENTS in mask with FAN_MARK_MOUNT are not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_FID),
		.mark = FLAGS_DESC(FAN_MARK_MOUNT),
		.mask = FLAGS_DESC(INODE_EVENTS),
		.expected_errno = EINVAL,
	},

	/* FAN_REPORT_NAME without FAN_REPORT_DIR_FID is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_NAME),
		.expected_errno = EINVAL,
	},

	/* FAN_REPORT_NAME without FAN_REPORT_DIR_FID is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_FID | FAN_REPORT_NAME),
		.expected_errno = EINVAL,
	},

	/* FAN_REPORT_TARGET_FID without FAN_REPORT_FID is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_TARGET_FID | FAN_REPORT_DFID_NAME),
		.expected_errno = EINVAL,
	},

	/* FAN_REPORT_TARGET_FID without FAN_REPORT_NAME is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_TARGET_FID | FAN_REPORT_DFID_FID),
		.expected_errno = EINVAL,
	},

	/* FAN_RENAME without FAN_REPORT_NAME is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_FID),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(FAN_RENAME),
		.expected_errno = EINVAL,
	},

	/* With FAN_MARK_ONLYDIR on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_ONLYDIR),
		.mask = FLAGS_DESC(FAN_OPEN),
		.expected_errno = ENOTDIR,
	},

	/* With FAN_REPORT_TARGET_FID, FAN_DELETE on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME_TARGET),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(FAN_DELETE),
		.expected_errno = ENOTDIR,
	},

	/* With FAN_REPORT_TARGET_FID, FAN_RENAME on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME_TARGET),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(FAN_RENAME),
		.expected_errno = ENOTDIR,
	},

	/* With FAN_REPORT_TARGET_FID, FAN_ONDIR on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME_TARGET),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(FAN_OPEN | FAN_ONDIR),
		.expected_errno = ENOTDIR,
	},

	/* With FAN_REPORT_TARGET_FID, FAN_EVENT_ON_CHILD on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME_TARGET),
		.mark = FLAGS_DESC(FAN_MARK_INODE),
		.mask = FLAGS_DESC(FAN_OPEN | FAN_EVENT_ON_CHILD),
		.expected_errno = ENOTDIR,
	},

	/* FAN_MARK_IGNORE_SURV with FAN_DELETE on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME),
		.mark = FLAGS_DESC(FAN_MARK_IGNORE_SURV),
		.mask = FLAGS_DESC(FAN_DELETE),
		.expected_errno = ENOTDIR,
	},

	/* FAN_MARK_IGNORE_SURV with FAN_RENAME on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME),
		.mark = FLAGS_DESC(FAN_MARK_IGNORE_SURV),
		.mask = FLAGS_DESC(FAN_RENAME),
		.expected_errno = ENOTDIR,
	},

	/* FAN_MARK_IGNORE_SURV with FAN_ONDIR on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME),
		.mark = FLAGS_DESC(FAN_MARK_IGNORE_SURV),
		.mask = FLAGS_DESC(FAN_OPEN | FAN_ONDIR),
		.expected_errno = ENOTDIR,
	},

	/* FAN_MARK_IGNORE_SURV with FAN_EVENT_ON_CHILD on non-dir is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME),
		.mark = FLAGS_DESC(FAN_MARK_IGNORE_SURV),
		.mask = FLAGS_DESC(FAN_OPEN | FAN_EVENT_ON_CHILD),
		.expected_errno = ENOTDIR,
	},

	/* FAN_MARK_IGNORE without FAN_MARK_IGNORED_SURV_MODIFY on directory is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_IGNORE),
		.mask = FLAGS_DESC(FAN_OPEN),
		.expected_errno = EISDIR,
	},

	/* FAN_MARK_IGNORE without FAN_MARK_IGNORED_SURV_MODIFY on mount mark is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_MOUNT | FAN_MARK_IGNORE),
		.mask = FLAGS_DESC(FAN_OPEN),
		.expected_errno = EINVAL,
	},

	/* FAN_MARK_IGNORE without FAN_MARK_IGNORED_SURV_MODIFY on filesystem mark is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_FILESYSTEM | FAN_MARK_IGNORE),
		.mask = FLAGS_DESC(FAN_OPEN),
		.expected_errno = EINVAL,
	},
	/* mount mark on anonymous pipe is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_MOUNT),
		.mask = { FAN_ACCESS, "anonymous pipe"},
		.pfd = pipes,
		.expected_errno = EINVAL,
	},
	/* filesystem mark on anonymous pipe is not valid */
	{
		.init = FLAGS_DESC(FAN_CLASS_NOTIF),
		.mark = FLAGS_DESC(FAN_MARK_FILESYSTEM),
		.mask = { FAN_ACCESS, "anonymous pipe"},
		.pfd = pipes,
		.expected_errno = EINVAL,
	},
};

static void do_test(unsigned int number)
{
	struct test_case_t *tc = &test_cases[number];

	tst_res(TINFO, "Test case %d: fanotify_init(%s, O_RDONLY)", number,
		tc->init.desc);

	if (fan_report_target_fid_unsupported && tc->init.flags & FAN_REPORT_TARGET_FID) {
		FANOTIFY_INIT_FLAGS_ERR_MSG(FAN_REPORT_TARGET_FID,
					    fan_report_target_fid_unsupported);
		return;
	}

	if (ignore_mark_unsupported && tc->mark.flags & FAN_MARK_IGNORE) {
		tst_res(TCONF, "FAN_MARK_IGNORE not supported in kernel?");
		return;
	}

	if (!tc->mask.flags && tc->expected_errno) {
		TST_EXP_FAIL(fanotify_init(tc->init.flags, O_RDONLY),
			tc->expected_errno);
	} else {
		TST_EXP_FD(fanotify_init(tc->init.flags, O_RDONLY));
	}

	fanotify_fd = TST_RET;

	if (fanotify_fd < 0)
		return;

	if (!tc->mask.flags)
		goto out;

	/* Set mark on non-dir only when expecting error ENOTDIR */
	const char *path = tc->expected_errno == ENOTDIR ? FILE1 : MNTPOINT;
	int dirfd = AT_FDCWD;

	if (tc->pfd) {
		dirfd = tc->pfd[0];
		path = NULL;
	}

	tst_res(TINFO, "Testing %s with %s",
		tc->mark.desc, tc->mask.desc);
	TST_EXP_FD_OR_FAIL(fanotify_mark(fanotify_fd, FAN_MARK_ADD | tc->mark.flags,
					 tc->mask.flags, dirfd, path),
					 tc->expected_errno);

	/*
	 * ENOTDIR are errors for events/flags not allowed on a non-dir inode.
	 * Try to set an inode mark on a directory and it should succeed.
	 * Try to set directory events in filesystem mark mask on non-dir
	 * and it should succeed.
	 */
	if (TST_PASS && tc->expected_errno == ENOTDIR) {
		SAFE_FANOTIFY_MARK(fanotify_fd, FAN_MARK_ADD | tc->mark.flags,
				   tc->mask.flags, AT_FDCWD, MNTPOINT);
		tst_res(TPASS,
			"Adding an inode mark on directory did not fail with "
			"ENOTDIR error as on non-dir inode");

		if (!(tc->mark.flags & FAN_MARK_ONLYDIR)) {
			SAFE_FANOTIFY_MARK(fanotify_fd, FAN_MARK_ADD | tc->mark.flags |
					   FAN_MARK_FILESYSTEM, tc->mask.flags,
					   AT_FDCWD, FILE1);
			tst_res(TPASS,
				"Adding a filesystem mark on non-dir did not fail with "
				"ENOTDIR error as with an inode mark");
		}
	}

out:
	if (fanotify_fd > 0)
		SAFE_CLOSE(fanotify_fd);
}

static void do_setup(void)
{
	/* Require FAN_REPORT_FID support for all tests to simplify per test case requirements */
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(FAN_REPORT_FID, MNTPOINT);

	fan_report_target_fid_unsupported =
		fanotify_init_flags_supported_on_fs(FAN_REPORT_DFID_NAME_TARGET, MNTPOINT);
	ignore_mark_unsupported = fanotify_mark_supported_by_kernel(FAN_MARK_IGNORE_SURV);

	/* Create temporary test file to place marks on */
	SAFE_FILE_PRINTF(FILE1, "0");
	/* Create anonymous pipes to place marks on */
	SAFE_PIPE2(pipes, O_CLOEXEC);
}

static void do_cleanup(void)
{
	if (fanotify_fd > 0)
		SAFE_CLOSE(fanotify_fd);
	if (pipes[0] != -1)
		SAFE_CLOSE(pipes[0]);
	if (pipes[1] != -1)
		SAFE_CLOSE(pipes[1]);
}

static struct tst_test test = {
	.needs_root = 1,
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = do_setup,
	.cleanup = do_cleanup,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ceaf69f8eadc"},
		{"linux-git", "8698e3bab4dd"},
		{"linux-git", "69562eb0bd3e"},
		{}
	}
};

#else
	TST_TEST_TCONF("System does not have required fanotify support");
#endif
