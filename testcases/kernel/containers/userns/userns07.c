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

#define MAXNEST 32

static void setup(void)
{
	check_newuser();
}

static int child_fn1(void *arg)
{
	pid_t cpid1;
	long level = (long)arg;
	int status;
	int parentuid;
	int parentgid;

	TST_CHECKPOINT_WAIT(0);

	if (level == MAXNEST) {
		tst_res(TPASS, "nested all children");
		return 0;
	}

	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, (void *)child_fn1, (void *)(level + 1));
	if (cpid1 < 0) {
		tst_res(TFAIL | TERRNO, "level %ld, unexpected error", level);
		return 1;
	}

	parentuid = geteuid();
	parentgid = getegid();

	updatemap(cpid1, UID_MAP, 0, parentuid);
	updatemap(cpid1, GID_MAP, 0, parentgid);

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAITPID(cpid1, &status, 0);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_brk(TBROK, "child %s", tst_strstatus(status));

	return 0;
}

static void run(void)
{
	pid_t cpid1;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];

	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, (void *)child_fn1, (void *)0);
	if (cpid1 < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

	parentuid = geteuid();
	parentgid = getegid();

	if (access("/proc/self/setgroups", F_OK) == 0) {
		sprintf(path, "/proc/%d/setgroups", cpid1);
		SAFE_FILE_PRINTF(path, "deny");
	}

	updatemap(cpid1, UID_MAP, 0, parentuid);
	updatemap(cpid1, GID_MAP, 0, parentgid);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};
