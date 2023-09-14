// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that the user ID and group ID, which are inside a container,
 * can be modified by its parent process.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include "tst_test.h"
#include "lapi/sched.h"

static void child_fn1(void)
{
	int uid, gid;

	TST_CHECKPOINT_WAIT(0);

	uid = geteuid();
	gid = getegid();

	TST_EXP_EQ_LI(uid, 100);
	TST_EXP_EQ_LI(gid, 100);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWUSER,
		.exit_signal = SIGCHLD,
	};
	int childpid;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];

	childpid = SAFE_CLONE(&args);
	if (!childpid) {
		child_fn1();
		return;
	}

	parentuid = geteuid();
	parentgid = getegid();

	sprintf(path, "/proc/%d/uid_map", childpid);
	SAFE_FILE_PRINTF(path, "100 %d 1", parentuid);

	if (access("/proc/self/setgroups", F_OK) == 0) {
		sprintf(path, "/proc/%d/setgroups", childpid);
		SAFE_FILE_PRINTF(path, "deny");
	}

	sprintf(path, "/proc/%d/gid_map", childpid);
	SAFE_FILE_PRINTF(path, "100 %d 1", parentgid);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};
