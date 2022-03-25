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
#include "common.h"

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void)
{
	TST_CHECKPOINT_WAIT(0);
	return 0;
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
	int cpid1, cpid2, cpid3;
	unsigned int parentuserns, cpid1userns, cpid2userns, newparentuserns;

	parentuserns = getusernsidbypid(getpid());

	cpid1 = ltp_clone_quick(SIGCHLD, (void *)child_fn1, NULL);
	if (cpid1 < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

	cpid1userns = getusernsidbypid(cpid1);

	TST_CHECKPOINT_WAKE(0);

	/* A process created via fork(2) or clone(2) without the
	 * CLONE_NEWUSER flag is a member of the same user namespace as its
	 * parent
	 */
	if (parentuserns != cpid1userns)
		tst_res(TFAIL, "userns:parent should be equal to cpid1");
	else
		tst_res(TPASS, "userns:parent is equal to cpid1");

	cpid2 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, (void *)child_fn1, NULL);
	if (cpid2 < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

	cpid2userns = getusernsidbypid(cpid2);

	TST_CHECKPOINT_WAKE(0);

	if (parentuserns == cpid2userns)
		tst_res(TFAIL, "userns:parent should be not equal to cpid2");
	else
		tst_res(TPASS, "userns:parent is not equal to cpid2");

	cpid3 = SAFE_FORK();
	if (!cpid3) {
		SAFE_UNSHARE(CLONE_NEWUSER);
		newparentuserns = getusernsidbypid(getpid());

		/* When unshare an user namespace, the calling process
		 * is moved into a new user namespace which is not shared
		 * with any previously existing process
		 */
		if (parentuserns == newparentuserns)
			tst_res(TFAIL, "unshared namespaces with same id");
		else
			tst_res(TPASS, "unshared namespaces with different id");
	}
}

static void setup(void)
{
	check_newuser();
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};
