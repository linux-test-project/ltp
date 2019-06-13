// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Porting from Crackerjack to LTP is done by
 * Manas Kumar Nayak maknayak@in.ibm.com>
 *
 * Waits for SIGALRM in rt_sigsuspend() then checks that process mask wasn't
 * modified.
 */

#include <signal.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/safe_rt_signal.h"
#include "lapi/signal.h"

static void sig_handler(int sig)
{
	(void) sig;
}

static void verify_rt_sigsuspend(void)
{
	int i;
	sigset_t set, set1, set2;
	struct sigaction act = {.sa_handler = sig_handler};

	if (sigemptyset(&set) < 0)
		tst_brk(TFAIL | TERRNO, "sigemptyset failed");

	SAFE_RT_SIGACTION(SIGALRM, &act, NULL, SIGSETSIZE);

	SAFE_RT_SIGPROCMASK(0, NULL, &set1, SIGSETSIZE);

	alarm(1);

	TEST(tst_syscall(__NR_rt_sigsuspend, &set, SIGSETSIZE));

	alarm(0);

	if (TST_RET != -1)
		tst_brk(TFAIL, "rt_sigsuspend returned %ld", TST_RET);

	if (TST_ERR != EINTR)
		tst_brk(TFAIL | TTERRNO, "rt_sigsuspend() failed unexpectedly");

	tst_res(TPASS, "rt_sigsuspend() returned with -1 and EINTR");

	SAFE_RT_SIGPROCMASK(0, NULL, &set2, SIGSETSIZE);
	for (i = 1; i < SIGRTMAX; i++) {
		if (i >= __SIGRTMIN && i < SIGRTMIN)
			continue;
		if (sigismember(&set1, i) != sigismember(&set2, i))
			tst_brk(TFAIL, "signal mask not preserved");
	}
	tst_res(TPASS, "signal mask preserved");
}

static struct tst_test test = {
	.test_all = verify_rt_sigsuspend,
};
