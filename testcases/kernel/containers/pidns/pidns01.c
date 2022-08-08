// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               27/12/07  Rishikesh K Rajak <risrajak@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWNS flag and check:
 *
 * - child process ID must be 1
 * - parent process ID must be 0
 */

#include "tst_test.h"
#include "lapi/namespaces_constants.h"

static int child_func(LTP_ATTRIBUTE_UNUSED void *arg)
{
	pid_t cpid, ppid;

	cpid = getpid();
	ppid = getppid();

	TST_EXP_PASS(cpid == 1);
	TST_EXP_PASS(ppid == 0);

	return 0;
}

static void run(void)
{
	int ret;

	ret = ltp_clone_quick(CLONE_NEWNS | SIGCHLD, child_func, NULL);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "clone failed");
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
};
