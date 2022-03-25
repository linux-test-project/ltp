// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that if a namespace isn't another namespace's ancestor, the process in
 * first namespace does not have the CAP_SYS_ADMIN capability in the second
 * namespace and the setns() call fails.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include "common.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

static void setup(void)
{
	check_newuser();
	tst_syscall(__NR_setns, -1, 0);
}

static int child_fn1(LTP_ATTRIBUTE_UNUSED void *arg)
{
	TST_CHECKPOINT_WAIT(0);
	return 0;
}

static int child_fn2(void *arg)
{
	TEST(tst_syscall(__NR_setns, ((long)arg), CLONE_NEWUSER));
	if (TST_RET != -1 || TST_ERR != EPERM)
		tst_res(TFAIL | TERRNO, "child2 setns() error");
	else
		tst_res(TPASS, "child2 setns() failed as expected");

	TST_CHECKPOINT_WAIT(1);

	return 0;
}

static void run(void)
{
	pid_t cpid1, cpid2, cpid3;
	char path[BUFSIZ];
	int fd;

	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, (void *)child_fn1, NULL);
	if (cpid1 < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

	sprintf(path, "/proc/%d/ns/user", cpid1);

	fd = SAFE_OPEN(path, O_RDONLY, 0644);
	cpid2 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, (void *)child_fn2, (void *)((long)fd));
	if (cpid2 < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

	/* child 3 - throw-away process changing ns to child1 */
	cpid3 = SAFE_FORK();
	if (!cpid3) {
		TST_EXP_PASS(tst_syscall(__NR_setns, fd, CLONE_NEWUSER));
		return;
	}

	TST_CHECKPOINT_WAKE(0);
	TST_CHECKPOINT_WAKE(1);

	SAFE_CLOSE(fd);
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
