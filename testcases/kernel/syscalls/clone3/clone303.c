// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 SUSE LLC <wegao@suse.com>
 */

/*\
 * This test case check clone3 CLONE_INTO_CGROUP flag
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/sched.h"
#include "lapi/pidfd.h"

#define BUF_LEN 20

static struct tst_cg_group *cg_child_test_simple;
static int fd;
static struct tst_clone_args *args;

static pid_t clone_into_cgroup(int cgroup_fd)
{

	args->flags = CLONE_INTO_CGROUP;
	args->exit_signal = SIGCHLD;
	args->cgroup = cgroup_fd;

	return tst_clone(args);
}

static void run(void)
{
	pid_t pid;

	pid = clone_into_cgroup(fd);

	if (!pid) {
		TST_CHECKPOINT_WAIT(0);
		return;
	}

	char buf[BUF_LEN];

	SAFE_CG_READ(cg_child_test_simple, "cgroup.procs", buf, BUF_LEN);

	if (atoi(buf) == pid)
		tst_res(TPASS, "clone3 case pass!");
	else
		tst_brk(TFAIL | TTERRNO, "clone3() failed !");

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAITPID(pid, NULL, 0);

}

static void setup(void)
{
	clone3_supported_by_kernel();

	cg_child_test_simple = tst_cg_group_mk(tst_cg, "cg_test_simple");

	fd = tst_cg_group_unified_dir_fd(cg_child_test_simple);

	if (fd < 0)
		tst_brk(TBROK, "get dir fd failed!");
}

static void cleanup(void)
{
	cg_child_test_simple = tst_cg_group_rm(cg_child_test_simple);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_cgroup_ctrls = (const char *const []){ "base", NULL },
	.needs_cgroup_ver = TST_CG_V2,
	.needs_checkpoints = 1,
	.min_kver = "5.7",
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{},
	}
};
