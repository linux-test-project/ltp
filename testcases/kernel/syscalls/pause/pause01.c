// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 * Copyright (c) 2025 Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that, pause() returns -1 and sets errno to EINTR after receipt of a
 * signal which is caught by the calling process.
 */

#include "tst_test.h"

static void sig_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void do_child(void)
{
	SAFE_SIGNAL(SIGINT, sig_handler);
	TST_CHECKPOINT_WAKE(0);
	TST_EXP_FAIL(pause(), EINTR);
	tst_res(TPASS, "Process resumed from pause()");
}

static void run(void)
{
	int pid, status;

	pid = SAFE_FORK();
	if (!pid) {
		do_child();
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_KILL(pid, SIGINT);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_all = run,
};
