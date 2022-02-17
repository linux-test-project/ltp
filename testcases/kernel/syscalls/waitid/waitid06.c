// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak maknayak@in.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test is checking if waitid() syscall filters children which exited.
 */

#include <sys/wait.h>
#include "tst_test.h"

static void run(void)
{
	siginfo_t infop;
	pid_t pid_child;

	/* dummy fork */
	pid_child = SAFE_FORK();
	if (!pid_child)
		return;

	tst_res(TINFO, "filter child by WEXITED");

	memset(&infop, 0, sizeof(infop));
	TST_EXP_PASS(waitid(P_PID, pid_child, &infop, WEXITED));

	tst_res(TINFO, "si_pid = %d ; si_code = %d ; si_status = %d",
		infop.si_pid, infop.si_code, infop.si_status);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
