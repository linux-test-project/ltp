// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Test whether the LTP library properly reaps any children left over when
 * the main test process dies. Run using test_children_cleanup.sh.
 */

#include <unistd.h>
#include <signal.h>

#include "tst_test.h"

static void run(void)
{
	pid_t child_pid, main_pid = getpid();

	tst_res(TINFO, "Main process %d starting", main_pid);

	/* Check that normal child reaping does not disrupt the test */
	if (!SAFE_FORK())
		return;

	SAFE_WAIT(NULL);
	child_pid = SAFE_FORK();

	/* Start child that will outlive the main test process */
	if (!child_pid) {
		sleep(30);
		return;
	}

	tst_res(TINFO, "Forked child %d", child_pid);
	kill(main_pid, SIGKILL);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
