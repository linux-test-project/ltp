// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2024
 * Author: Ma Xinjian <maxj.fnst@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that getcpu(2) fails with EFAULT if cpu_id or node_id points outside
 * the calling process address space.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/sched.h"

static unsigned int cpu_id, node_id;

static struct tcase {
	unsigned int *cpu_id;
	unsigned int *node_id;
	char *desc;
} tcases[] = {
	{NULL, &node_id, "cpu_id"},
	{&cpu_id, NULL, "node_id"},
};

static void check_getcpu(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int status;
	pid_t pid;

	tst_res(TINFO, "Test %s outside process address space", tc->desc);

	if (!tc->cpu_id)
		tc->cpu_id = tst_get_bad_addr(NULL);

	if (!tc->node_id)
		tc->node_id = tst_get_bad_addr(NULL);

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_FAIL(getcpu(tc->cpu_id, tc->node_id), EFAULT);

		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "getcpu() caused SIGSEGV");
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	tst_res(TFAIL, "child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test = check_getcpu,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
};
