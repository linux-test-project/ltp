// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that if a namespace isn't another namespace's ancestor, the process in
 * first namespace does not have the CAP_SYS_ADMIN capability in the second
 * namespace and the setns() call fails.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include "tst_test.h"
#include "lapi/sched.h"

static void child_fn1(void)
{
	TST_CHECKPOINT_WAIT(0);
}

static void child_fn2(int fd)
{
	TST_EXP_FAIL(setns(fd, CLONE_NEWUSER), EPERM);
	TST_CHECKPOINT_WAIT(1);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWUSER,
		.exit_signal = SIGCHLD,
	};
	pid_t cpid1, cpid2, cpid3;
	char path[BUFSIZ];
	int fd;

	cpid1 = SAFE_CLONE(&args);
	if (!cpid1) {
		child_fn1();
		return;
	}

	sprintf(path, "/proc/%d/ns/user", cpid1);

	fd = SAFE_OPEN(path, O_RDONLY, 0644);

	cpid2 = SAFE_CLONE(&args);
	if (!cpid2) {
		child_fn2(fd);
		return;
	}

	cpid3 = SAFE_FORK();
	if (!cpid3) {
		TST_EXP_PASS(setns(fd, CLONE_NEWUSER));
		return;
	}

	TST_CHECKPOINT_WAKE(0);
	TST_CHECKPOINT_WAKE(1);

	SAFE_CLOSE(fd);
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
