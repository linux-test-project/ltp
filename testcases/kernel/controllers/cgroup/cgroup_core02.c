// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * When a task is writing to an fd opened by a different task, the perm check
 * should use the cgroup namespace of the latter task.
 *
 * It is copy from kernel selftests cgroup test_core test_cgcore_lesser_ns_open
 * subcase. Note that this case only runs on cgroup2 as cgroup1 doesn't have
 * namespace support.
 *
 * It is a regression test for
 *
 * commit e57457641613fef0d147ede8bd6a3047df588b95
 * Author: Tejun Heo <tj@kernel.org>
 * Date:   Thu Jan 6 11:02:29 2022 -1000
 *
 * cgroup: Use open-time cgroup namespace for process migration perm checks
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_file_at.h"

static struct tst_cg_group *cg_child_a, *cg_child_b;
static uid_t nobody_uid;

struct lesser_ns_open_thread_arg {
	int fds[TST_CG_ROOTS_MAX];
	int loops;
};

static int lesser_ns_open_thread_fn(void *arg)
{
	struct lesser_ns_open_thread_arg *targ = arg;

	targ->loops = SAFE_CG_OPEN(cg_child_b, "cgroup.procs", O_RDWR, targ->fds);
	return 0;
}

static void test_lesser_ns_open(void)
{
	int i;
	static char stack[65536];
	pid_t pid;
	int status;
	struct lesser_ns_open_thread_arg targ = { .fds = {0}, .loops = -1};

	cg_child_a = tst_cg_group_mk(tst_cg, "child_a");
	cg_child_b = tst_cg_group_mk(tst_cg, "child_b");

	if (!SAFE_FORK()) {
		SAFE_CG_PRINT(cg_child_a, "cgroup.procs", "0");
		SAFE_CG_FCHOWN(cg_child_a, "cgroup.procs",  nobody_uid, -1);
		SAFE_CG_FCHOWN(cg_child_b, "cgroup.procs",  nobody_uid, -1);
		pid  = ltp_clone(CLONE_NEWCGROUP | CLONE_FILES | CLONE_VM | SIGCHLD,
					lesser_ns_open_thread_fn, &targ, 65536, stack);
		if (pid < 0)  {
			tst_res(TFAIL, "unexpected negative pid %d", pid);
			exit(1);
		}

		SAFE_WAITPID(pid, &status, 0);
		for (i = 0; i < targ.loops; i++) {
			if (targ.fds[i] < 1) {
				tst_res(TFAIL, "unexpected negative fd %d", targ.fds[i]);
				exit(1);
			}

			TEST(write(targ.fds[i], "0", 1));
			if (TST_RET >= 0 || TST_ERR != ENOENT)
				tst_res(TFAIL, "%s failed", __func__);
			else
				tst_res(TPASS | TTERRNO, "%s passed", __func__);

			SAFE_CLOSE(targ.fds[i]);
		}
		exit(0);
	}

	tst_reap_children();
	cg_child_b = tst_cg_group_rm(cg_child_b);
	cg_child_a = tst_cg_group_rm(cg_child_a);
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;
}

static void cleanup(void)
{
	if (cg_child_a) {
		SAFE_CG_PRINTF(tst_cg_drain, "cgroup.procs", "%d", getpid());
		cg_child_a = tst_cg_group_rm(cg_child_a);
	}
	if (cg_child_b) {
		SAFE_CG_PRINTF(tst_cg_drain, "cgroup.procs", "%d", getpid());
		cg_child_b = tst_cg_group_rm(cg_child_b);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_lesser_ns_open,
	.forks_child = 1,
	.needs_root = 1,
	.needs_cgroup_ctrls = (const char *const[]){"memory",  NULL},
	.needs_cgroup_ver = TST_CG_V2,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e57457641613"},
		{"CVE", "2021-4197"},
		{}
	},
};
