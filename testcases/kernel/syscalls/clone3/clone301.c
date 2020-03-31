// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic clone3() test.
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/clone.h"
#include "lapi/pidfd_send_signal.h"

#define CHILD_SIGNAL	SIGUSR1
#define DATA	777

static int pidfd, child_tid, parent_tid, parent_received_signal;
static volatile int child_received_signal;
static struct clone_args *args;

static struct tcase {
	uint64_t flags;
	int exit_signal;
} tcases[] = {
	{0, SIGCHLD},
	{0, SIGUSR2},
	{CLONE_FS, SIGCHLD},
	{CLONE_NEWPID, SIGCHLD},
	{CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | CLONE_PIDFD, SIGCHLD},
};

static void parent_rx_signal(int sig)
{
	parent_received_signal = sig;
}

static void child_rx_signal(int sig, siginfo_t *info, void *ucontext)
{
	(void) ucontext;

	if (sig == CHILD_SIGNAL && info && info->si_value.sival_int == DATA)
		child_received_signal = 1;
}

static struct sigaction psig_action = {
	.sa_handler = parent_rx_signal,
};

static struct sigaction csig_action = {
	.sa_sigaction = child_rx_signal,
	.sa_flags = SA_SIGINFO,
};

static siginfo_t uinfo = {
	.si_signo = CHILD_SIGNAL,
	.si_code = SI_QUEUE,
	.si_value.sival_int = DATA,
};


static void do_child(int clone_pidfd, int n)
{
	int count = 1000;

	if (clone_pidfd) {
		child_received_signal = 0;
		SAFE_SIGACTION(CHILD_SIGNAL, &csig_action, NULL);

		TST_CHECKPOINT_WAKE(0);

		while(!child_received_signal && --count)
			usleep(100);

		if (child_received_signal)
			tst_res(TPASS, "clone3() passed: Child received correct signal (index %d)", n);
		else
			tst_res(TFAIL, "clone3() failed: Child received incorrect signal (index %d)", n);
	}

	exit(0);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int status, clone_pidfd = tc->flags & CLONE_PIDFD;
	pid_t pid;

	args->flags = tc->flags;
	args->pidfd = (uint64_t)(&pidfd);
	args->child_tid = (uint64_t)(&child_tid);
	args->parent_tid = (uint64_t)(&parent_tid);
	args->exit_signal = tc->exit_signal;
	args->stack = 0;
	args->stack_size = 0;
	args->tls = 0;

	TEST(pid = clone3(args, sizeof(*args)));
	if (pid < 0) {
		tst_res(TFAIL | TTERRNO, "clone3() failed (%d)", n);
		return;
	}

	parent_received_signal = 0;
	SAFE_SIGACTION(tc->exit_signal, &psig_action, NULL);

	if (!pid)
		do_child(clone_pidfd, n);

	/* Need to send signal to child process */
	if (clone_pidfd) {
		TST_CHECKPOINT_WAIT(0);

		TEST(pidfd_send_signal(pidfd, CHILD_SIGNAL, &uinfo, 0));
		if (TST_RET != 0) {
			tst_res(TFAIL | TTERRNO, "pidfd_send_signal() failed");
			return;
		}
	}

	SAFE_WAITPID(pid, &status, __WALL);

	if (parent_received_signal == tc->exit_signal)
		tst_res(TPASS, "clone3() passed: Parent received correct signal (index %d)", n);
	else
		tst_res(TFAIL, "clone3() failed: Parent received incorrect signal (index %d)", n);
}

static void setup(void)
{
	clone3_supported_by_kernel();

	uinfo.si_pid = getpid();
	uinfo.si_uid = getuid();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.needs_checkpoints = 1,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{},
	}
};
