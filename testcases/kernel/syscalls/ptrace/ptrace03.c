// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * 1) ptrace() returns -1 and sets errno to ESRCH if process with
 * specified pid does not exist.
 * 2) ptrace() returns -1 and sets errno to EPERM if we are trying
 * to trace a process which is already been traced
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "tst_test.h"

static pid_t unused_pid;
static pid_t zero_pid;

static struct tcase {
	int request;
	pid_t *pid;
	int exp_errno;
	char *tname;
} tcases[] = {
	{PTRACE_ATTACH, &unused_pid, ESRCH,
	"Trace a process which does not exist"},

	{PTRACE_TRACEME, &zero_pid, EPERM,
	"Trace a process which is already been traced"}
};

static void verify_ptrace(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int child_pid;

	tst_res(TINFO, "%s", tc->tname);

	child_pid = SAFE_FORK();
	if (!child_pid) {
		if (tc->exp_errno == EPERM)
			SAFE_PTRACE(PTRACE_TRACEME, 0, NULL, NULL);

		TEST(ptrace(tc->request, *(tc->pid), NULL, NULL));
		if (TST_RET == 0) {
			tst_res(TFAIL, "ptrace() succeeded unexpectedly");
			exit(1);
		}
		if (tc->exp_errno == TST_ERR)
			tst_res(TPASS | TTERRNO, "ptrace() failed as expected");
		else
			tst_res(TFAIL | TTERRNO, "ptrace() failed unexpectedly, expected %s",
					tst_strerrno(tc->exp_errno));
	}
	tst_reap_children();
}

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
}

static struct tst_test test = {
	.test = verify_ptrace,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.forks_child = 1,
};
