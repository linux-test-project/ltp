// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: Bob Clark, Barrie Kletscher
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks following conditions:
 *
 * 1. sighold action to turn off the receipt of all signals was done without error.
 * 2. After signals were held, and sent, no signals were trapped.
 */

#define _XOPEN_SOURCE 600
#include <signal.h>
#include "tst_test.h"

#ifndef NSIG
#	define NSIG _NSIG
#endif

#ifndef NUMSIGS
#	define NUMSIGS NSIG
#endif

static int sigs_catched;
static int sigs_map[NUMSIGS];

static int skip_sig(int sig)
{
	if (sig >= 32 && sig < SIGRTMIN)
		return 1;

	switch (sig) {
	case SIGCHLD:
	case SIGKILL:
	case SIGALRM:
	case SIGSTOP:
		return 1;
	default:
		return 0;
	}
}

static void handle_sigs(int sig)
{
	sigs_map[sig] = 1;
	sigs_catched++;
}

static void do_child(void)
{
	int sig;

	for (sig = 1; sig < NUMSIGS; sig++) {
		if (skip_sig(sig))
			continue;

		SAFE_SIGNAL(sig, handle_sigs);
	}

	for (sig = 1; sig < NUMSIGS; sig++) {
		if (skip_sig(sig))
			continue;

		if (sighold(sig))
			tst_brk(TBROK | TERRNO, "sighold(%s %i)", tst_strsig(sig), sig);
	}

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (!sigs_catched) {
		tst_res(TPASS, "all signals were hold");
		return;
	}

	tst_res(TFAIL, "signal handler was executed");

	for (sig = 1; sig < NUMSIGS; sig++)
		if (sigs_map[sig])
			tst_res(TINFO, "Signal %i(%s) catched", sig, tst_strsig(sig));
}

static void run(void)
{
	pid_t pid_child;
	int signal;

	pid_child = SAFE_FORK();
	if (!pid_child) {
		do_child();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	for (signal = 1; signal < NUMSIGS; signal++) {
		if (skip_sig(signal))
			continue;

		SAFE_KILL(pid_child, signal);
	}

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
