// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test PR_SET_CHILD_SUBREAPER and PR_GET_CHILD_SUBREAPER of prctl(2).
 *
 * - If PR_SET_CHILD_SUBREAPER marks a process as a child subreaper, it
 *   fulfills the role of init(1) for its descendant orphaned process.
 *   The PPID of its orphaned process will be reparented to the subreaper
 *   process, and the subreaper process can receive a SIGCHLD signal and
 *   wait(2) on the orphaned process to discover corresponding termination
 *   status.
 * - The setting of PR_SET_CHILD_SUBREAPER is not inherited by children
 *   reated by fork(2).
 * - PR_GET_CHILD_SUBREAPER can get the setting of PR_SET_CHILD_SUBREAPER.
 *
 * These flags was added by kernel commit ebec18a6d3aa:
 * "prctl: add PR_{SET,GET}_CHILD_SUBREAPER to allow simple process supervision"
 */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>

#include "tst_test.h"
#include "lapi/prctl.h"

static volatile int sigchild_recv;

static void check_get_subreaper(int exp_val)
{
	int get_val;

	TEST(prctl(PR_GET_CHILD_SUBREAPER, &get_val));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "prctl(PR_GET_CHILD_SUBREAPER) failed");
		return;
	}

	if (get_val == exp_val) {
		tst_res(TPASS, "prctl(PR_GET_CHILD_SUBREAPER) got expected %d",
			get_val);
	} else {
		tst_res(TFAIL, "prctl(PR_GET_CHILD_SUBREAPER) got %d, expected %d",
			get_val, exp_val);
	}
}

static void verify_prctl(void)
{
	int status, ret;
	pid_t pid;
	pid_t ppid = getpid();

	sigchild_recv = 0;

	TEST(prctl(PR_SET_CHILD_SUBREAPER, 1));
	if (TST_RET == -1) {
		if (TST_ERR == EINVAL) {
			tst_res(TCONF,
				"prctl() doesn't support PR_SET_CHILD_SUBREAPER");
		} else {
			tst_res(TFAIL | TTERRNO,
				"prctl(PR_SET_CHILD_SUBREAPER) failed");
		}
		return;
	}

	tst_res(TPASS, "prctl(PR_SET_CHILD_SUBREAPER) succeeded");

	pid = SAFE_FORK();
	if (!pid) {
		pid_t cpid;

		cpid = SAFE_FORK();
		if (!cpid) {
			TST_CHECKPOINT_WAIT(0);
			if (getppid() != ppid) {
				tst_res(TFAIL,
					"PPID of orphaned process was not reparented");
				exit(0);
			}

			tst_res(TPASS, "PPID of orphaned process was reparented");
			exit(0);
		}

		check_get_subreaper(0);
		exit(0);
	}

	SAFE_WAITPID(pid, NULL, 0);
	TST_CHECKPOINT_WAKE(0);
	ret = wait(&status);
	if (ret > 0) {
		tst_res(TPASS, "wait() got orphaned process, pid %d, status %d",
			ret, status);
	} else {
		tst_res(TFAIL | TERRNO,
			"wait() failed to get orphaned process");
	}

	if (sigchild_recv == 2)
		tst_res(TPASS, "received SIGCHLD from orphaned process");
	else
		tst_res(TFAIL, "didn't receive SIGCHLD from orphaned process");

	check_get_subreaper(1);
}

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	sigchild_recv++;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGCHLD, sighandler);
}

static struct tst_test test = {
	.setup = setup,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_all = verify_prctl,
};
