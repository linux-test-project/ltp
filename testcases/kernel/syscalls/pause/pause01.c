/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Check that pause() returns on signal with errno == EINTR.
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
	if (TEST_RETURN != -1)
		tst_res(TFAIL, "pause() succeeded unexpectedly");
	else if (TEST_ERRNO == EINTR)
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
	TST_PROCESS_STATE_WAIT(pid, 'S');
	kill(pid, SIGINT);

	/*
	 * TST_CHECKPOINT_WAIT has built-in timeout, if pause() doesn't return,
	 * this checkpoint call will reliably end the test.
	 */
	TST_CHECKPOINT_WAIT(0);
	SAFE_WAIT(&status);
}

static struct tst_test test = {
	.tid = "pause01",
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_all = do_test,
};
