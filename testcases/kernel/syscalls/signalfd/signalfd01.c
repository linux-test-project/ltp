// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Red Hat Inc., 2008
 * Copyright (c) Linux Test Project, 2006-2024
 */

/*\
 * [Description]
 *
 * Verify that signalfd() works as expected.
 *
 * - signalfd() can create fd, and fd can receive signal.
 * - signalfd() can reassign fd, and fd can receive signal.
 */

#include <sys/signalfd.h>
#include "tst_test.h"

static int fd_signal = -1;
static sigset_t mask1;
static sigset_t mask2;

static void check_signal(int fd, uint32_t signal)
{
	pid_t pid = getpid();
	ssize_t bytes;
	struct signalfd_siginfo siginfo;

	SAFE_KILL(pid, signal);
	bytes = SAFE_READ(0, fd, &siginfo, sizeof(siginfo));
	TST_EXP_EQ_LI(bytes, sizeof(siginfo));
	TST_EXP_EQ_LI(siginfo.ssi_signo, signal);
}

static void setup(void)
{
	SAFE_SIGEMPTYSET(&mask1);
	SAFE_SIGADDSET(&mask1, SIGUSR1);
	SAFE_SIGPROCMASK(SIG_BLOCK, &mask1, NULL);
	SAFE_SIGEMPTYSET(&mask2);
	SAFE_SIGADDSET(&mask2, SIGUSR2);
	SAFE_SIGPROCMASK(SIG_BLOCK, &mask2, NULL);
}

static void cleanup(void)
{
	if (fd_signal > 0)
		SAFE_CLOSE(fd_signal);
}

static void verify_signalfd(void)
{
	/* create fd */
	TST_EXP_FD(signalfd(fd_signal, &mask1, 0),
		"%s", "signalfd() create fd");
	if (TST_RET == -1)
		return;
	fd_signal = TST_RET;
	check_signal(fd_signal, SIGUSR1);
	/* reassign fd */
	TST_EXP_FD(signalfd(fd_signal, &mask2, 0), "%s",
		"signalfd() reassign fd");
	if (TST_RET == -1)
		return;
	TST_EXP_EQ_LI(TST_RET, fd_signal);
	check_signal(fd_signal, SIGUSR2);
}

static struct tst_test test = {
	.test_all = verify_signalfd,
	.setup = setup,
	.cleanup = cleanup,
};
