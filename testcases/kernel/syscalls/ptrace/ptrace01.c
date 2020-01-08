// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2019 SUSE LLC
 *
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 * Ported to new library: Jorik Cronenberg <jcronenberg@suse.de>
 *
 * Test the functionality of ptrace() for PTRACE_TRACEME in combination with
 * PTRACE_KILL and PTRACE_CONT requests.
 * Forked child does ptrace(PTRACE_TRACEME, ...).
 * Then a signal is delivered to the child and verified that parent
 * is notified via wait().
 * Afterwards parent does ptrace(PTRACE_KILL, ..)/ptrace(PTRACE_CONT, ..)
 * and then parent does wait() for child to finish.
 * Test passes if child exits with SIGKILL for PTRACE_KILL.
 * Test passes if child exits normally for PTRACE_CONT.
 *
 * Testing two cases for each:
 * 1) child ignore SIGUSR2 signal
 * 2) using a signal handler for child for SIGUSR2
 * In both cases, child should stop & notify parent on reception of SIGUSR2.
 */

#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <config.h>
#include "ptrace.h"
#include "tst_test.h"

static struct tcase {
	int handler;
	int request;
	int exp_wifexited;
	int exp_wtermsig;
	char *message;
} tcases[] = {
	{0, PTRACE_KILL, 0, 9, "Testing PTRACE_KILL without child handler"},
	{1, PTRACE_KILL, 0, 9, "Testing PTRACE_KILL with child handler"},
	{0, PTRACE_CONT, 1, 0, "Testing PTRACE_CONT without child handler"},
	{1, PTRACE_CONT, 1, 0, "Testing PTRACE_CONT with child handler"},
};

static volatile int got_signal;

static void child_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	SAFE_KILL(getppid(), SIGUSR2);
}

static void parent_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	got_signal = 1;
}

static void do_child(unsigned int i)
{
	struct sigaction child_act;

	if (i == 0)
		child_act.sa_handler = SIG_IGN;
	else
		child_act.sa_handler = child_handler;

	child_act.sa_flags = SA_RESTART;
	sigemptyset(&child_act.sa_mask);

	SAFE_SIGACTION(SIGUSR2, &child_act, NULL);

	if ((ptrace(PTRACE_TRACEME, 0, 0, 0)) == -1) {
		tst_res(TWARN, "ptrace() failed in child");
		exit(1);
	}
	SAFE_KILL(getpid(), SIGUSR2);
	exit(1);
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	pid_t child_pid;
	int status;
	struct sigaction parent_act;

	got_signal = 0;

	tst_res(TINFO, tc->message);

	if (tc->handler == 1) {
		parent_act.sa_handler = parent_handler;
		parent_act.sa_flags = SA_RESTART;
		sigemptyset(&parent_act.sa_mask);
		SAFE_SIGACTION(SIGUSR2, &parent_act, NULL);
	}

	child_pid = SAFE_FORK();

	if (!child_pid)
		do_child(tc->handler);

	SAFE_WAITPID(child_pid, &status, 0);

	if (((WIFEXITED(status)) && (WEXITSTATUS(status)))
		 || (got_signal == 1))
		tst_res(TFAIL, "Test Failed");
	else if ((ptrace(tc->request, child_pid, 0, 0)) == -1)
		tst_res(TFAIL | TERRNO, "ptrace(%i, %i, 0, 0) failed",
			tc->request, child_pid);

	SAFE_WAITPID(child_pid, &status, 0);

	if ((tc->request == PTRACE_CONT &&
	     WIFEXITED(status) && WEXITSTATUS(status) == tc->exp_wifexited) ||
	    (tc->request == PTRACE_KILL &&
	     WIFSIGNALED(status) && WTERMSIG(status) == tc->exp_wtermsig)) {
		tst_res(TPASS, "Child %s as expected", tst_strstatus(status));
	} else {
		tst_res(TFAIL, "Child %s unexpectedly", tst_strstatus(status));
	}

}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
};
