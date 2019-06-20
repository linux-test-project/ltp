// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Matthew Bobrowski. All Rights Reserved.
 *
 * Started by Matthew Bobrowski <mbobrowski@mbobrowski.org>
 *
 * DESCRIPTION
 *	This test file has been designed to ensure that the fanotify
 *	system calls fanotify_init(2) and fanotify_mark(2) return the
 *	correct error code to the calling process when an invalid flag or
 *	mask value has been specified in conjunction with FAN_REPORT_FID.
 */
#define _GNU_SOURCE
#include "tst_test.h"
#include "fanotify.h"

#include <errno.h>

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define MNTPOINT "mntpoint"
#define FILE1 MNTPOINT"/file1"

/*
 * List of inode events that are only available when notification group is
 * set to report fid.
 */
#define INODE_EVENTS (FAN_ATTRIB | FAN_CREATE | FAN_DELETE | FAN_MOVE | \
		      FAN_DELETE_SELF | FAN_MOVE_SELF)

static int fanotify_fd;

/*
 * Each test case has been designed in a manner whereby the values defined
 * within should result in the interface to return an error to the calling
 * process.
 */
static struct test_case_t {
	unsigned int init_flags;
	unsigned int mark_flags;
	unsigned long long mask;
} test_cases[] = {
	{
		FAN_CLASS_CONTENT | FAN_REPORT_FID, 0, 0
	},
	{
		FAN_CLASS_PRE_CONTENT | FAN_REPORT_FID, 0, 0
	},
	{
		FAN_CLASS_NOTIF, 0, INODE_EVENTS
	},
	{
		FAN_CLASS_NOTIF | FAN_REPORT_FID, FAN_MARK_MOUNT, INODE_EVENTS
	}
};

static void do_test(unsigned int number)
{
	int ret;
	struct test_case_t *tc = &test_cases[number];

	fanotify_fd = fanotify_init(tc->init_flags, O_RDONLY);
	if (fanotify_fd < 0) {
		/*
		 * EINVAL is to be returned to the calling process when
		 * an invalid notification class is specified in
		 * conjunction with FAN_REPORT_FID.
		 */
		if (errno == EINVAL) {
			tst_res(TPASS,
				"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) "
				"failed with error EINVAL as expected",
				fanotify_fd,
				tc->init_flags);
			return;
		}
		tst_brk(TBROK | TERRNO,
			"fanotify_fd=%d, fanotify_init(%x, O_RDONLY) failed",
			fanotify_fd,
			tc->init_flags);
	}

	/*
	 * A test case with a mask set to zero indicate that they've been
	 * specifically designed to test and fail on the fanotify_init()
	 * system call.
	 */
	if (tc->mask == 0) {
		tst_res(TFAIL,
			"fanotify_fd=%d fanotify_init(%x, O_RDONLY) "
			"unexpectedly succeeded when tests with mask 0 are"
			"expected to fail when calling fanotify_init()",
			fanotify_fd,
			tc->init_flags);
		goto out;
	}

	ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD | tc->mark_flags,
				tc->mask, AT_FDCWD, FILE1);
	if (ret < 0) {
		/*
		 * EINVAL is to be returned to the calling process when
		 * attempting to use INODE_EVENTS without FAN_REPORT_FID
		 * specified on the notification group, or using
		 * INODE_EVENTS with mark type FAN_MARK_MOUNT.
		 */
		if (errno == EINVAL) {
			tst_res(TPASS,
				"ret=%d, fanotify_mark(%d, FAN_MARK_ADD | %x, "
				"%llx, AT_FDCWD, %s) failed with error EINVAL "
				"as expected",
				ret,
				fanotify_fd,
				tc->mark_flags,
				tc->mask,
				FILE1);
			goto out;
		}
		tst_brk(TBROK | TERRNO,
			"ret=%d, fanotify_mark(%d, FAN_MARK_ADD | %x, %llx, "
			"AT_FDCWD, %s) failed",
			ret,
			fanotify_fd,
			tc->mark_flags,
			tc->mask,
			FILE1);
	}

	tst_res(TFAIL,
		"fanotify_fd=%d, ret=%d, fanotify_init(%x, O_RDONLY) and "
		"fanotify_mark(%d, FAN_MARK_ADD | %x, %llx, AT_FDCWD, %s) did "
		"not return any errors as expected",
		fanotify_fd,
		ret,
		tc->init_flags,
		fanotify_fd,
		tc->mark_flags,
		tc->mask,
		FILE1);
out:
	SAFE_CLOSE(fanotify_fd);
}

static void do_setup(void)
{
	int fd;

	/* Check for kernel fanotify support */
	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);
	SAFE_CLOSE(fd);

	/* Create temporary test file to place marks on */
	SAFE_FILE_PRINTF(FILE1, "0");
}

static void do_cleanup(void)
{
	if (fanotify_fd > 0)
		SAFE_CLOSE(fanotify_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = do_setup,
	.cleanup = do_cleanup,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1
};

#else
	TST_TEST_TCONF("System does not have required fanotify support");
#endif
