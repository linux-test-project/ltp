/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 John George - Ported
 *  04/2002 wjhuie sigset cleanups
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
 */

/*
 * Tests to see if pids returned from fork and waitpid are same
 *
 * Fork 8 kids, 2 will immediately exit, 2 will sleep, 2 will be compute bound
 * and 2 will fork/reap a child for 50 times.
 */

#include "waitpid_common.h"

static void do_compute(void);
static void do_fork(void);
static void do_sleep(void);

static void do_child_1(void)
{
	pid_t pid;
	int i;

	for (i = 0; i < MAXKIDS; i++) {
		pid = SAFE_FORK();
		if (pid == 0) {
			if (i == 0 || i == 1)
				do_exit(0);

			if (i == 2 || i == 3)
				do_compute();

			if (i == 4 || i == 5)
				do_fork();

			if (i == 6 || i == 7)
				do_sleep();
		}

		fork_kid_pid[i] = pid;
	}

	TST_CHECKPOINT_WAKE2(0, MAXKIDS);

	if (TST_TRACE(reap_children(0, 0, fork_kid_pid, MAXKIDS)))
		return;

	tst_res(TPASS, "Test PASSED");
}

static void do_compute(void)
{
	int i;

	TST_CHECKPOINT_WAIT(0);

	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;

	exit(3);
}

static void do_fork(void)
{
	pid_t fork_pid;
	int i;

	TST_CHECKPOINT_WAIT(0);

	for (i = 0; i < 50; i++) {
		fork_pid = SAFE_FORK();
		if (fork_pid == 0)
			exit(3);

		if (TST_TRACE(reap_children(fork_pid, 0, &fork_pid, 1)))
			break;
	}

	exit(3);
}

static void do_sleep(void)
{
	TST_CHECKPOINT_WAIT(0);

	sleep(1);
	sleep(1);

	exit(3);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.setup = waitpid_setup,
	.cleanup = waitpid_cleanup,
	.test_all = waitpid_test,
};
