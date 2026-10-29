// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2009-2021
 * Copyright (c) Crackerjack Project, 2007
 * Ported from Crackerjack to LTP by Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * Basic tests for the tkill syscall.
 *
 * [Algorithm]
 *
 * Calls tkill and capture signal to verify success.
 */

#include <signal.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static volatile sig_atomic_t sig_flag;

static void sighandler(int sig)
{
	if (sig == SIGUSR1)
		sig_flag = 1;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGUSR1, sighandler);
}

static void run(void)
{
	int tid;
	int timeout_ms = 1000;
	sig_flag = 0;

	tid = tst_syscall(__NR_gettid);
	TST_EXP_PASS(tst_syscall(__NR_tkill, tid, SIGUSR1));

	while (timeout_ms--) {
		if (sig_flag)
			break;

		usleep(1000);
	}

	if (sig_flag)
		tst_res(TPASS, "signal captured");
	else
		tst_res(TFAIL, "signal not captured");
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = run,
};
