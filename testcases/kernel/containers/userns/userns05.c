// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that if a process created via fork(2) or clone(2) without the
 * CLONE_NEWUSER flag is a member of the same user namespace as its parent.
 *
 * When unshare an user namespace, the calling process is moved into a new user
 * namespace which is not shared with any previously existing process.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include "tst_test.h"
#include "lapi/sched.h"
#include "common.h"

static void child_fn1(void)
{
	TST_CHECKPOINT_WAIT(0);
}

static unsigned int getusernsidbypid(int pid)
{
	char path[BUFSIZ];
	char userid[BUFSIZ];
	unsigned int id = 0;

	sprintf(path, "/proc/%d/ns/user", pid);

	SAFE_READLINK(path, userid, BUFSIZ);

	if (sscanf(userid, "user:[%u]", &id) < 0)
		tst_brk(TBROK | TERRNO, "sscanf failure");

	return id;
}

static void run(void)
{
	const struct tst_clone_args args1 = { .exit_signal = SIGCHLD };
	const struct tst_clone_args args2 = { CLONE_NEWUSER, SIGCHLD };
	int cpid1, cpid2, cpid3;
	unsigned int parentuserns, cpid1userns, cpid2userns, newparentuserns;

	parentuserns = getusernsidbypid(getpid());

	cpid1 = SAFE_CLONE(&args1);
	if (!cpid1) {
		child_fn1();
		return;
	}

	cpid1userns = getusernsidbypid(cpid1);

	TST_CHECKPOINT_WAKE(0);

	/* A process created via fork(2) or clone(2) without the
	 * CLONE_NEWUSER flag is a member of the same user namespace as its
	 * parent
	 */
	TST_EXP_EQ_LI(parentuserns, cpid1userns);

	cpid2 = SAFE_CLONE(&args2);
	if (!cpid2) {
		child_fn1();
		return;
	}

	cpid2userns = getusernsidbypid(cpid2);

	TST_CHECKPOINT_WAKE(0);

	TST_EXP_EXPR(parentuserns != cpid2userns,
		"parent namespace != child namespace");

	cpid3 = SAFE_FORK();
	if (!cpid3) {
		SAFE_UNSHARE(CLONE_NEWUSER);
		newparentuserns = getusernsidbypid(getpid());

		/* When unshare an user namespace, the calling process
		 * is moved into a new user namespace which is not shared
		 * with any previously existing process
		 */
		TST_EXP_EXPR(parentuserns != newparentuserns,
			"parent namespace != unshared child namespace");
	}
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
