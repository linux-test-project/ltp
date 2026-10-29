// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Modernized checkpoint usage.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_checkpoint.h"

/* Test 1: Basic checkpoint signal from child to parent */
static void checkpoint_test1(void)
{
	pid_t pid = SAFE_FORK();

	if (pid == 0) {
		tst_res(TINFO, "Child: signaling checkpoint");
		TST_CHECKPOINT_WAKE(0);
		_exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	tst_res(TPASS, "Parent: checkpoint reached");

	SAFE_WAITPID(pid, NULL, 0);

	return;
}

/* Test 2: Checkpoint wait with timeout, wake from child */
static void checkpoint_test2(void)
{
	pid_t pid = SAFE_FORK();

	if (pid == 0) {
		tst_res(TINFO, "Child: signaling checkpoint");
		TST_CHECKPOINT_WAKE2(0, 1);
		_exit(0);
	}

	TST_CHECKPOINT_WAIT2(0, 1000);
	tst_res(TPASS, "Parent: checkpoint reached");

	SAFE_WAITPID(pid, NULL, 0);

	return;
}

/* Test 3: Wake two child waiters on the same checkpoint */
static void checkpoint_test3(void)
{
	pid_t pid1, pid2;

	pid1 = SAFE_FORK();
	if (pid1 == 0) {
		tst_res(TINFO, "Child 1: waiting on checkpoint 0 (no timeout)");
		TST_CHECKPOINT_WAIT(0);
		_exit(0);
	}

	pid2 = SAFE_FORK();
	if (pid2 == 0) {
		tst_res(TINFO, "Child 2: waiting on checkpoint 0 (1000ms timeout)");
		TST_CHECKPOINT_WAIT2(0, 1000);
		_exit(0);
	}

	TST_CHECKPOINT_WAKE2(0, 2);
	tst_res(TPASS, "Parent: checkpoint wake issued");

	tst_reap_children();

	return;
}

/* Test 4: Two-way checkpoint handshake (child->parent->child) */
static void checkpoint_test4(void)
{
	pid_t pid = SAFE_FORK();

	if (pid == 0) {
		tst_res(TINFO, "Child: waking and then waiting on checkpoint 0");
		TST_CHECKPOINT_WAKE_AND_WAIT(0);
		_exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	TST_CHECKPOINT_WAKE(0);

	tst_res(TPASS, "Parent: checkpoint handshake completed");

	SAFE_WAITPID(pid, NULL, 0);

	return;
}

static void run(void)
{
	checkpoint_test1();
	checkpoint_test2();
	checkpoint_test3();
	checkpoint_test4();

	return;
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
