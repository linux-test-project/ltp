// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Fujitsu Ltd.
 * Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*
 * This is a regression test about the race in autogroup, this test
 * can crash the buggy kernel, and the bug has been fixed in:
 *
 *   commit 18f649ef344127ef6de23a5a4272dbe2fdb73dde
 *   Author: Oleg Nesterov <oleg@redhat.com>
 *   Date:   Mon Nov 14 19:46:09 2016 +0100
 *
 *   sched/autogroup: Fix autogroup_move_group() to never skip sched_move_task()
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "tst_test.h"

#define LOOPS	1000
#define PATH_AUTOGROUP	"/proc/sys/kernel/sched_autogroup_enabled"

static int orig_autogroup = -1;

static void do_test(void)
{
	int i;

	if (!SAFE_FORK()) {
		SAFE_FILE_PRINTF(PATH_AUTOGROUP, "%d", 1);
		SAFE_SETSID();

		if (SAFE_FORK())
			pause();

		SAFE_KILL(getppid(), SIGKILL);
		usleep(1000);

		// The child has gone, the grandchild runs with kref == 1
		SAFE_FILE_PRINTF(PATH_AUTOGROUP, "%d", 0);
		SAFE_SETSID();

		// runs with the freed ag/tg
		for (i = 0; i < LOOPS; i++)
			usleep(10);

		TST_CHECKPOINT_WAKE(0);

		exit(0);
	}

	SAFE_WAIT(NULL); // destroy the child's ag/tg

	TST_CHECKPOINT_WAIT(0);

	tst_res(TPASS, "Bug not reproduced");
}

static void setup(void)
{
	if (access(PATH_AUTOGROUP, F_OK))
		tst_brk(TCONF, "autogroup not supported");

	SAFE_FILE_SCANF(PATH_AUTOGROUP, "%d", &orig_autogroup);
}

static void cleanup(void)
{
	if (orig_autogroup != -1)
		SAFE_FILE_PRINTF(PATH_AUTOGROUP, "%d", orig_autogroup);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "18f649ef3441"},
		{}
	}
};
