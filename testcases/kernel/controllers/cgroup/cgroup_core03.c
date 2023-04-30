// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2012 Christian Brauner <brauner-AT-kernel.org>
 * Copyright (c) 2023 SUSE LLC <wegao@suse.com>
 */

/*\
 * [Description]
 *
 * This test is copied from kselftest
 * tools/testing/selftests/cgroup/test_kill.c.
 *
 * Only simple test implemented within current case, the other cases such
 * as test_cgkill_tree and test_cgkill_forkbomb can be created later.
 *
 */

#include <sys/wait.h>

#include "lapi/syscalls.h"
#include "tst_test.h"

#define MAX_PID_NUM 100
#define PID_NUM MIN(MAX_PID_NUM, (tst_ncpus_available() + 1))
#define BUF_LEN (20 * PID_NUM)

static int *data_ptr;
static char *buf;
static struct tst_cg_group *cg_child_test_simple;

static int wait_for_pid(pid_t pid)
{
	int status, ret;

again:
	ret = waitpid(pid, &status, 0);
	if (ret == -1) {
		if (errno == EINTR)
			goto again;

		return -1;
	}

	if (WIFSIGNALED(status))
		return 0;

	return -1;
}

static int cg_run_nowait(const struct tst_cg_group *const cg)
{
	int pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		SAFE_CG_PRINTF(cg, "cgroup.procs", "%d", getpid());
		if (tst_atomic_inc(data_ptr) == PID_NUM)
			TST_CHECKPOINT_WAKE(0);
		pause();
	}

	return pid;
}

static int cg_count_procs(const struct tst_cg_group *cg)
{
	char *ptr;

	int nr = 0;

	SAFE_CG_READ(cg, "cgroup.procs", buf, BUF_LEN);

	for (ptr = buf; *ptr; ptr++)
		if (*ptr == '\n')
			nr++;

	return nr;
}

static void run(void)
{
	pid_t pids[MAX_PID_NUM];
	int i;
	*data_ptr = 0;

	cg_child_test_simple = tst_cg_group_mk(tst_cg, "cg_test_simple");

	if (!SAFE_CG_HAS(cg_child_test_simple, "cgroup.kill")) {
		cg_child_test_simple = tst_cg_group_rm(cg_child_test_simple);
		tst_brk(TCONF, "cgroup.kill is not supported on your distribution");
	}

	memset(buf, 0, BUF_LEN);

	for (i = 0; i < PID_NUM; i++)
		pids[i] = cg_run_nowait(cg_child_test_simple);

	TST_CHECKPOINT_WAIT(0);
	TST_EXP_VAL(cg_count_procs(cg_child_test_simple), PID_NUM);
	SAFE_CG_PRINTF(cg_child_test_simple, "cgroup.kill", "%d", 1);

	for (i = 0; i < PID_NUM; i++)
		TST_EXP_PASS_SILENT(wait_for_pid(pids[i]));

	TST_EXP_VAL(cg_count_procs(cg_child_test_simple), 0);
	cg_child_test_simple = tst_cg_group_rm(cg_child_test_simple);
}

static void setup(void)
{
	buf = tst_alloc(BUF_LEN);
	data_ptr = SAFE_MMAP(NULL, sizeof(uintptr_t), PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (data_ptr)
		SAFE_MUNMAP(data_ptr, sizeof(uintptr_t));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.max_runtime = 20,
	.needs_cgroup_ctrls = (const char *const []){ "base", NULL },
	.needs_cgroup_ver = TST_CG_V2,
	.needs_checkpoints = 1,
};
