// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 *
 * tgkill() should fail with EAGAIN when RLIMIT_SIGPENDING is reached with a
 * real-time signal.  Test this by starting a child thread with SIGRTMIN
 * blocked and a limit of 0 pending signals, then attempting to deliver
 * SIGRTMIN from the parent thread.
 */

#include <pthread.h>
#include <signal.h>

#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tgkill.h"

static void *thread_func(void *arg)
{
	const struct rlimit sigpending = {
		.rlim_cur = 0,
		.rlim_max = 0,
	};
	sigset_t sigrtmin;
	int err;
	pid_t *tid = arg;

	sigemptyset(&sigrtmin);
	sigaddset(&sigrtmin, SIGRTMIN);

	err = pthread_sigmask(SIG_BLOCK, &sigrtmin, NULL);
	if (err)
		tst_brk(TBROK, "pthread_sigmask() failed: %s",
			tst_strerrno(err));

	SAFE_SETRLIMIT(RLIMIT_SIGPENDING, &sigpending);
	*tid = sys_gettid();

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	return arg;
}

static void run(void)
{
	pthread_t thread;
	pid_t tid = -1;

	SAFE_PTHREAD_CREATE(&thread, NULL, thread_func, &tid);

	TST_CHECKPOINT_WAIT(0);

	TEST(sys_tgkill(getpid(), tid, SIGRTMIN));
	if (TST_RET && TST_ERR == EAGAIN)
		tst_res(TPASS, "tgkill() failed with EAGAIN as expected");
	else
		tst_res(TFAIL | TTERRNO,
			"tgkill() should have failed with EAGAIN");

	TST_CHECKPOINT_WAKE(0);

	SAFE_PTHREAD_JOIN(thread, NULL);
}

static struct tst_test test = {
	.needs_checkpoints = 1,
	.test_all = run,
};
