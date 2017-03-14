/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset testx for SIG_ERR not < 0, TPASS|TFAIL issued
 *      04/2002 wjhuie sigset cleanups
 */

/*
 * DESCRIPTION
 *	Tests to see if pids returned from fork and waitpid are same
 *
 * ALGORITHM
 *	Check proper functioning of waitpid with pid = 0 and < -1 with arg
 *	WNOHANG
 */

#include "waitpid_common.h"

static void do_child_1(void)
{
	pid_t pid, group;
	int i;
	int status;

	group = SAFE_GETPGID(0);

	for (i = 0; i < MAXKIDS; i++) {
		if (i == (MAXKIDS / 2))
			SAFE_SETPGID(0, 0);

		pid = SAFE_FORK();
		if (pid == 0)
			do_exit(0);

		fork_kid_pid[i] = pid;
	}

	if (TST_TRACE(waitpid_ret_test(0, &status, WNOHANG, 0, 0)))
		return;

	if (TST_TRACE(waitpid_ret_test(-group, &status, WNOHANG, 0, 0)))
		return;

	TST_CHECKPOINT_WAKE2(0, MAXKIDS);

	if (TST_TRACE(reap_children(0, WNOHANG, fork_kid_pid + (MAXKIDS / 2),
				    MAXKIDS / 2)))
		return;

	if (TST_TRACE(reap_children(-group, WNOHANG, fork_kid_pid,
				    MAXKIDS / 2)))
		return;

	tst_res(TPASS, "Test PASSED");
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.setup = waitpid_setup,
	.cleanup = waitpid_cleanup,
	.test_all = waitpid_test,
};
