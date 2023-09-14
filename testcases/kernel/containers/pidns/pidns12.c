// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 *               13/11/08  Gowrishankar M <gowrishankar.m@in.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and verifies that siginfo->si_pid is
 * set to 0 if sender (parent process) is not in the receiver's namespace.
 */

#include "tst_test_macros.h"
#define _GNU_SOURCE 1
#include <signal.h>
#include "tst_test.h"
#include "lapi/sched.h"

static volatile pid_t sig_pid = -1;

static void child_signal_handler(LTP_ATTRIBUTE_UNUSED int sig, siginfo_t *si, LTP_ATTRIBUTE_UNUSED void *unused)
{
	sig_pid = si->si_pid;
}

static void child_func(void)
{
	struct sigaction sa;

	TST_EXP_EQ_LI(tst_getpid(), 1);
	TST_EXP_EQ_LI(getppid(), 0);

	sa.sa_flags = SA_SIGINFO;
	SAFE_SIGFILLSET(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;

	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	TST_EXP_EQ_LI(sig_pid, 0);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};
	int pid;

	pid = SAFE_CLONE(&args);
	if (!pid) {
		child_func();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	SAFE_KILL(pid, SIGUSR1);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
