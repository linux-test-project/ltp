// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) 2021 CTERA Networks.  All Rights Reserved.
 *
 * User ns support by: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Forked from getxattr05.c by Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * Check that fanotify groups and marks limits are enforced correctly.
 * If user ns is supported, verify that global limit and per user ns
 * limits are both enforced.
 * Otherwise, we only check that global groups limit is enforced.
 */

#define _GNU_SOURCE
#include "config.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/sched.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define MOUNT_PATH	"fs_mnt"
#define TEST_FILE	MOUNT_PATH "/testfile"
#define SELF_USERNS	"/proc/self/ns/user"
#define MAX_USERNS	"/proc/sys/user/max_user_namespaces"
#define UID_MAP		"/proc/self/uid_map"

#define GLOBAL_MAX_GROUPS "/proc/sys/fs/fanotify/max_user_groups"
#define GLOBAL_MAX_MARKS  "/proc/sys/fs/fanotify/max_user_marks"
#define USERNS_MAX_GROUPS "/proc/sys/user/max_fanotify_groups"
#define USERNS_MAX_MARKS  "/proc/sys/user/max_fanotify_marks"

/*
 * In older kernels those limits were fixed in kernel.
 * The fanotify_init() man page documents the max groups limit is 128, but the
 * implementation actually allows one extra group.
 */
#define DEFAULT_MAX_GROUPS 129
#define DEFAULT_MAX_MARKS  8192

static int orig_max_userns = -1;
static int user_ns_supported = 1;
static int max_groups = DEFAULT_MAX_GROUPS;
static int max_marks = DEFAULT_MAX_MARKS;

static struct tcase {
	const char *tname;
	unsigned int init_flags;
	/* 0: without userns, 1: with userns */
	int set_userns;
	/* 0: don't map root UID in userns, 1: map root UID in userns */
	int map_root;
	/* 0: unlimited groups in userns */
	int max_user_groups;
	/* 0: unlimited marks in userns */
	int max_user_marks;
} tcases[] = {
	{
		"Global groups limit in init user ns",
		FAN_CLASS_NOTIF,
		0, 0, 0, 0,
	},
	{
		"Global groups limit in privileged user ns",
		FANOTIFY_REQUIRED_USER_INIT_FLAGS,
		1, 1, 0, 0,
	},
	{
		"Local groups limit in unprivileged user ns",
		FANOTIFY_REQUIRED_USER_INIT_FLAGS,
		1, 0, 10, 0,
	},
	{
		"Local marks limit in unprivileged user ns",
		FANOTIFY_REQUIRED_USER_INIT_FLAGS,
		1, 0, 0, 10,
	},
};

/* Verify that groups and marks cannot be created beyond limit */
static void verify_user_limits(unsigned int init_flags, int groups, int marks)
{
	int i, fd = 0, ret = 0;

	for (i = 0; i <= groups; i++) {
		fd = fanotify_init(init_flags, O_RDONLY);
		/*
		 * Don't bother closing fd's, the child process will exit
		 * and all fd's will be closed.
		 */
		if (fd < 0)
			break;

		ret = fanotify_mark(fd, FAN_MARK_ADD, FAN_OPEN, AT_FDCWD,
				    TEST_FILE);
		if (ret < 0)
			break;

	}
	if (fd > 0 && i > groups) {
		tst_res(TFAIL,
			"Created %d groups and marks - "
			"groups limit (%d) exceeded", i, groups);
	} else if (!ret && i > marks) {
		tst_res(TFAIL,
			"Created %d groups and marks - "
			"marks limit (%d) exceeded", i, marks);
	} else if (ret < 0 && errno == ENOSPC && marks < groups) {
		/*
		 * ENOSPC is to be returned to the calling process when
		 * fanotify marks limit is reached.
		 */
		tst_res(TPASS,
			"Created %d marks - "
			"below marks limit (%d)", i, marks);
	} else if (fd < 0 && errno == EMFILE) {
		/*
		 * EMFILE is to be returned to the calling process when
		 * fanotify groups limit is reached.
		 */
		tst_res(TPASS,
			"Created %d groups - "
			"below groups limit (%d)", i, groups);
	} else if (errno == EPERM) {
		tst_res(TCONF,
			"unprivileged fanotify not supported by kernel?");
	} else if (fd < 0) {
		tst_brk(TBROK | TERRNO,
			"fd=%d, fanotify_init(%x, O_RDONLY) failed",
			fd, init_flags);
	} else if (ret < 0) {
		tst_brk(TBROK | TERRNO,
			"ret=%d, fanotify_mark(%d, FAN_MARK_ADD, FAN_OPEN, "
			"AT_FDCWD, '" TEST_FILE "') failed", ret, fd);
	}
}

static void do_unshare(int map_root)
{
	int res;

	/* unshare() should support CLONE_NEWUSER flag since Linux 3.8 */
	res = unshare(CLONE_NEWUSER);
	if (res == -1)
		tst_brk(TFAIL | TERRNO, "unshare(CLONE_NEWUSER) failed");

	if (map_root) {
		/*
		 * uid_map file should exist since Linux 3.8 because
		 * it is available on Linux 3.5
		 */
		SAFE_ACCESS(UID_MAP, F_OK);

		SAFE_FILE_PRINTF(UID_MAP, "%d %d %d", 0, 0, 1);
	}
}

static void test_fanotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int groups = max_groups;
	int marks = max_marks;
	pid_t pid;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (tc->set_userns && !user_ns_supported) {
		tst_res(TCONF, "fanotify inside user namespace is not supported");
		return;
	}

	pid = SAFE_FORK();
	if (!pid) {
		if (tc->set_userns) {
			do_unshare(tc->map_root);
			/* Not changing global limits, only per userns limits */
			if (tc->max_user_groups && tc->max_user_groups < groups) {
				/* Further limit user ns groups */
				marks = groups = tc->max_user_groups;
				SAFE_FILE_PRINTF(USERNS_MAX_GROUPS, "%d", groups);
			}
			if (tc->max_user_marks && tc->max_user_marks < marks) {
				/* Further limit user ns marks */
				marks = tc->max_user_marks;
				SAFE_FILE_PRINTF(USERNS_MAX_MARKS, "%d", marks);
			}
		}
		verify_user_limits(tc->init_flags, groups, marks);
		exit(0);
	}

	tst_reap_children();
}

static void setup_rlimit(unsigned int max_files)
{
	struct rlimit rlim;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim);
	rlim.rlim_cur = max_files;
	SAFE_SETRLIMIT(RLIMIT_NOFILE, &rlim);
}

static void setup(void)
{
	SAFE_TOUCH(TEST_FILE, 0666, NULL);
	/* Check for kernel fanotify support */
	REQUIRE_FANOTIFY_INIT_FLAGS_SUPPORTED_ON_FS(FAN_REPORT_FID, TEST_FILE);

	/*
	 * The default value of max_user_namespaces is set to 0 on some distros,
	 * We need to change the default value to call unshare().
	 */
	if (access(SELF_USERNS, F_OK) != 0) {
		user_ns_supported = 0;
	} else if (!access(MAX_USERNS, F_OK)) {
		SAFE_FILE_SCANF(MAX_USERNS, "%d", &orig_max_userns);
		SAFE_FILE_PRINTF(MAX_USERNS, "%d", 10);
	}

	/*
	 * In older kernels those limits were fixed in kernel and fanotify is
	 * not permitted inside user ns.
	 */
	if (access(GLOBAL_MAX_GROUPS, F_OK) && errno == ENOENT) {
		user_ns_supported = 0;
	} else {
		SAFE_FILE_SCANF(GLOBAL_MAX_GROUPS, "%d", &max_groups);
		SAFE_FILE_SCANF(GLOBAL_MAX_MARKS, "%d", &max_marks);
	}
	tst_res(TINFO, "max_fanotify_groups=%d max_fanotify_marks=%d",
		max_groups, max_marks);

	/* Make sure we are not limited by nr of open files */
	setup_rlimit(max_groups * 2);
}

static void cleanup(void)
{
	if (orig_max_userns != -1)
		SAFE_FILE_PRINTF(MAX_USERNS, "%d", orig_max_userns);
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
};
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
