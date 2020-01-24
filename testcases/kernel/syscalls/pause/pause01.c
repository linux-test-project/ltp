// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "tst_test.h"

static void sig_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void do_child(void)
{
	SAFE_SIGNAL(SIGINT, sig_handler);

	TST_CHECKPOINT_WAKE(0);

	TEST(pause());
	if (TST_RET != -1)
		tst_res(TFAIL, "pause() succeeded unexpectedly");
	else if (TST_ERR == EINTR)
		tst_res(TPASS, "pause() interrupted with EINTR");
	else
		tst_res(TFAIL | TTERRNO, "pause() unexpected errno");

	TST_CHECKPOINT_WAKE(0);
	exit(0);
}

static void do_test(void)
{
	int pid, status;

	pid = SAFE_FORK();
	if (pid == 0)
		do_child();

	TST_CHECKPOINT_WAIT(0);
	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	kill(pid, SIGINT);

	/*
	 * TST_CHECKPOINT_WAIT has built-in timeout, if pause() doesn't return,
	 * this checkpoint call will reliably end the test.
	 */
	TST_CHECKPOINT_WAIT(0);
	SAFE_WAIT(&status);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_all = do_test,
};
