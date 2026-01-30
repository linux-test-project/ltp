// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               04/11/08  Veerendra C  <vechandr@in.ibm.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone a process with CLONE_NEWPID flag and verifies that siginfo->si_pid is
 * set to 0 if sender (parent process) sent the signal. Then send signal from
 * container itself and check if siginfo->si_pid is set to 1.
 */

#define _GNU_SOURCE 1
#include <signal.h>
#include "tst_test.h"
#include "lapi/sched.h"

static volatile int signal_pid;

static void child_signal_handler(LTP_ATTRIBUTE_UNUSED int sig, siginfo_t *si, LTP_ATTRIBUTE_UNUSED void *unused)
{
	signal_pid = si->si_pid;
}

static void child_func(void)
{
	struct sigaction sa;
	pid_t cpid, ppid;

	cpid = tst_getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	tst_res(TINFO, "Catching SIGUSR1 signal");

	sa.sa_flags = SA_SIGINFO;
	SAFE_SIGFILLSET(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	TST_EXP_EQ_LI(signal_pid, 0);

	tst_res(TINFO, "Sending SIGUSR1 from container itself");

	SAFE_KILL(cpid, SIGUSR1);

	TST_EXP_EQ_LI(signal_pid, 1);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};
	pid_t pid;

	signal_pid = -1;

	pid = SAFE_CLONE(&args);
	if (!pid) {
		child_func();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Sending SIGUSR1 from parent");

	SAFE_KILL(pid, SIGUSR1);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
