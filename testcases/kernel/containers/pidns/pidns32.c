// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check for the maxium amount of
 * nested containers.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"
#include "tst_atomic.h"
#include "lapi/sched.h"

#define MAXNEST 32

static const struct tst_clone_args args = {
	.flags = CLONE_NEWPID,
	.exit_signal = SIGCHLD,
};
static int *level;

static pid_t child_func(void)
{
	pid_t cpid = 0;

	if (tst_atomic_inc(level) == MAXNEST)
		return cpid;

	cpid = SAFE_CLONE(&args);
	if (!cpid) {
		child_func();
		return cpid;
	}

	tst_reap_children();

	return cpid;
}

static void setup(void)
{
	level = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(level, sizeof(int));
}

static void run(void)
{
	*level = 0;

	if (!child_func())
		return;

	TST_EXP_EQ_LI(*level, MAXNEST);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
