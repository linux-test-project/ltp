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
#include "common.h"
#include "tst_test.h"

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int uid, gid;

	TST_CHECKPOINT_WAIT(0);

	uid = geteuid();
	gid = getegid();

	if (uid == 100 && gid == 100)
		tst_res(TPASS, "got expected uid and gid");
	else
		tst_res(TFAIL, "got unexpected uid=%d gid=%d", uid, gid);

	return 0;
}

static void setup(void)
{
	check_newuser();
}

static void run(void)
{
	int childpid;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];

	childpid = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, child_fn1, NULL);
	if (childpid < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");

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
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};
