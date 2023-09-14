// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that when a process with non-zero user IDs performs an execve(),
 * the process's capability sets are cleared.
 * When a process with zero user IDs performs an execve(), the process's
 * capability sets are set.
 */

#include "tst_test.h"
#include "config.h"

#ifdef HAVE_LIBCAP
#define _GNU_SOURCE

#include <stdio.h>
#include "lapi/sched.h"
#include "common.h"

#define TEST_APP "userns06_capcheck"

#define CHILD1UID 0
#define CHILD1GID 0
#define CHILD2UID 200
#define CHILD2GID 200

static void child_fn1(void)
{
	char *const args[] = { TEST_APP, "privileged", NULL };
	int ret;

	TST_CHECKPOINT_WAIT(0);

	ret = execv(args[0], args);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "execv: unexpected error");
}

static void child_fn2(void)
{
	int uid, gid, ret;
	char *const args[] = { TEST_APP, "unprivileged", NULL };

	TST_CHECKPOINT_WAIT(1);

	uid = geteuid();
	gid = getegid();

	TST_EXP_EQ_LI(uid, CHILD2UID);
	TST_EXP_EQ_LI(gid, CHILD2GID);

	ret = execv(args[0], args);
	if (ret == -1)
		tst_brk(TBROK | TERRNO, "execv: unexpected error");
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWUSER,
		.exit_signal = SIGCHLD,
	};
	pid_t cpid1;
	pid_t cpid2;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];
	int fd;

	parentuid = geteuid();
	parentgid = getegid();

	cpid1 = SAFE_CLONE(&args);
	if (!cpid1) {
		child_fn1();
		return;
	}

	cpid2 = SAFE_CLONE(&args);
	if (!cpid2) {
		child_fn2();
		return;
	}

	if (access("/proc/self/setgroups", F_OK) == 0) {
		sprintf(path, "/proc/%d/setgroups", cpid1);

		fd = SAFE_OPEN(path, O_WRONLY, 0644);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "deny", 4);
		SAFE_CLOSE(fd);

		sprintf(path, "/proc/%d/setgroups", cpid2);

		fd = SAFE_OPEN(path, O_WRONLY, 0644);
		SAFE_WRITE(SAFE_WRITE_ALL, fd, "deny", 4);
		SAFE_CLOSE(fd);
	}

	updatemap(cpid1, UID_MAP, CHILD1UID, parentuid);
	updatemap(cpid2, UID_MAP, CHILD2UID, parentuid);

	updatemap(cpid1, GID_MAP, CHILD1GID, parentgid);
	updatemap(cpid2, GID_MAP, CHILD2GID, parentgid);

	TST_CHECKPOINT_WAKE(0);
	TST_CHECKPOINT_WAKE(1);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.resource_files = (const char *[]) {
		TEST_APP,
		NULL,
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};

#else
TST_TEST_TCONF("System is missing libcap");
#endif
