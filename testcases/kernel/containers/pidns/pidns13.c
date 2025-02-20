// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 * Copyright (c) SUSE LLC 2021
 *
 * History:
 * 23/10/08  Gowrishankar M 			Created test scenarion.
 *            <gowrishankar.m@in.ibm.com>
 */

/*\
 * The pidns13.c testcase checks container init, for async I/O
 * triggered by peer namespace process.
 *
 * [Algorithm]
 *
 * * create a pipe in parent namespace
 * * create two PID namespace containers(cinit1 and cinit2)
 * * in cinit1, set pipe read end to send SIGUSR1 for asynchronous I/O
 * * let cinit2 trigger async I/O on pipe write end
 * * in signal info, check si_code to be POLL_IN and si_fd to be pipe read fd
 */

#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_clone.h"
#include "lapi/sched.h"

static int pipe_fd[2];

#define CHILD_PID       1
#define PARENT_PID      0

static void child_signal_handler(int sig, siginfo_t *si,
				 void *unused LTP_ATTRIBUTE_UNUSED)
{
	tst_res(TWARN, "cinit(pid %d): Caught signal! sig=%d, si_fd=%d, si_code=%d",
		tst_getpid(), sig, si->si_fd, si->si_code);
}

static void child_fn(unsigned int cinit_no)
{
	struct sigaction sa;
	sigset_t newset;
	siginfo_t info;
	struct timespec timeout;
	pid_t pid, ppid;
	int flags;

	pid = tst_getpid();
	ppid = getppid();
	if (pid != CHILD_PID || ppid != PARENT_PID)
		tst_brk(TBROK, "cinit%u: pidns not created.", cinit_no);

	if (cinit_no == 1) {
		SAFE_CLOSE(pipe_fd[1]);

		sigemptyset(&newset);
		sigaddset(&newset, SIGUSR1);
		SAFE_SIGPROCMASK(SIG_BLOCK, &newset, NULL);

		SAFE_FCNTL(pipe_fd[0], F_SETOWN, pid);
		SAFE_FCNTL(pipe_fd[0], F_SETSIG, SIGUSR1);
		flags = SAFE_FCNTL(pipe_fd[0], F_GETFL);
		SAFE_FCNTL(pipe_fd[0], F_SETFL, flags | O_ASYNC);

		sa.sa_flags = SA_SIGINFO;
		sigfillset(&sa.sa_mask);
		sa.sa_sigaction = child_signal_handler;
		SAFE_SIGACTION(SIGUSR1, &sa, NULL);

		TST_CHECKPOINT_WAKE(1);

		timeout.tv_sec = 10;
		timeout.tv_nsec = 0;

		if (sigtimedwait(&newset, &info, &timeout) != SIGUSR1) {
			tst_brk(TBROK | TERRNO,
				"cinit1: sigtimedwait() failed.");
		}

		if (info.si_fd == pipe_fd[0] && info.si_code == POLL_IN)
			tst_res(TPASS, "cinit1: si_fd is %d, si_code is %d",
				info.si_fd, info.si_code);
		else
			tst_res(TFAIL, "cinit1: si_fd is %d, si_code is %d",
				info.si_fd, info.si_code);
	} else {
		SAFE_CLOSE(pipe_fd[0]);

		TST_CHECKPOINT_WAIT(1);
		SAFE_WRITE(SAFE_WRITE_ALL, pipe_fd[1], "test\n", 5);
	}

	exit(0);
}

static void run(void)
{
	const struct tst_clone_args cargs = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};

	SAFE_PIPE(pipe_fd);

	if (!SAFE_CLONE(&cargs))
		child_fn(1);

	if (!SAFE_CLONE(&cargs))
		child_fn(2);

	SAFE_CLOSE(pipe_fd[0]);
	SAFE_CLOSE(pipe_fd[1]);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
};
