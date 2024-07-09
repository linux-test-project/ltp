// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2003-2024
 */

/*\
 * [Description]
 *
 * Verify the basic sigsuspend(2) syscall functionality:
 *
 * - sigsuspend(2) can replace process's current signal mask by the specified
 *   signal mask and suspend the process execution until the delivery of a
 *   signal.
 * - sigsuspend(2) should return after the execution of signal handler and
 *   restore the previous signal mask.
 */

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "tst_test.h"

static sigset_t signalset, sigset1, sigset2;
static volatile sig_atomic_t alarm_num;

static void sig_handler(int sig)
{
	alarm_num = sig;
}

static void verify_sigsuspend(void)
{
	alarm_num = 0;

	SAFE_SIGFILLSET(&sigset2);

	alarm(1);

	/* Unblock SIGALRM */
	TEST(sigsuspend(&signalset));

	alarm(0);

	if (TST_RET != -1 || TST_ERR != EINTR) {
		tst_res(TFAIL | TTERRNO,
			"sigsuspend() returned value %ld", TST_RET);
		return;
	}

	if (alarm_num != SIGALRM) {
		tst_res(TFAIL, "sigsuspend() didn't unblock SIGALRM");
		return;
	}

	SAFE_SIGPROCMASK(0, NULL, &sigset2);
	if (memcmp(&sigset1, &sigset2, sizeof(unsigned long))) {
		tst_res(TFAIL, "sigsuspend() failed to "
			"restore the previous signal mask");
		return;
	}

	tst_res(TPASS, "sigsuspend() succeeded");
}

static void setup(void)
{
	SAFE_SIGEMPTYSET(&signalset);
	SAFE_SIGEMPTYSET(&sigset1);
	SAFE_SIGADDSET(&sigset1, SIGALRM);

	struct sigaction sa_new = {
		.sa_handler = sig_handler,
	};

	SAFE_SIGACTION(SIGALRM, &sa_new, 0);

	/* Block SIGALRM */
	SAFE_SIGPROCMASK(SIG_SETMASK, &sigset1, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_sigsuspend,
};
