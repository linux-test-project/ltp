// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that the kernel allows at least 32 nested levels of user namespaces.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include "common.h"
#include "tst_test.h"
#include "lapi/sched.h"

#define MAXNEST 32

static void child_fn1(const int level)
{
	const struct tst_clone_args args = { CLONE_NEWUSER, SIGCHLD };
	pid_t cpid;
	int parentuid;
	int parentgid;

	TST_CHECKPOINT_WAIT(0);

	if (level == MAXNEST) {
		tst_res(TPASS, "nested all children");
		return;
	}

	cpid = SAFE_CLONE(&args);
	if (!cpid) {
		child_fn1(level + 1);
		return;
	}

	parentuid = geteuid();
	parentgid = getegid();

	updatemap(cpid, UID_MAP, 0, parentuid);
	updatemap(cpid, GID_MAP, 0, parentgid);

	TST_CHECKPOINT_WAKE(0);

	tst_reap_children();
}

static void run(void)
{
	const struct tst_clone_args args = { CLONE_NEWUSER, SIGCHLD };
	pid_t cpid;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];

	cpid = SAFE_CLONE(&args);
	if (!cpid) {
		child_fn1(0);
		return;
	}

	parentuid = geteuid();
	parentgid = getegid();

	if (access("/proc/self/setgroups", F_OK) == 0) {
		sprintf(path, "/proc/%d/setgroups", cpid);
		SAFE_FILE_PRINTF(path, "deny");
	}

	updatemap(cpid, UID_MAP, 0, parentuid);
	updatemap(cpid, GID_MAP, 0, parentgid);

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
