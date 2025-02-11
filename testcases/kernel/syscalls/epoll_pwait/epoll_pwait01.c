// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Basic test for epoll_pwait() and epoll_pwait2().
 *
 * - With a sigmask a signal is ignored and the syscall safely waits until
 *   either a file descriptor becomes ready or the timeout expires.
 *
 * - Without sigmask if signal arrives a syscall is iterrupted by a signal.
 *   The call should return -1 and set errno to EINTR.
 */

#include <stdlib.h>
#include <sys/epoll.h>

#include "tst_test.h"
#include "epoll_pwait_var.h"

static int efd, sfd[2];
static struct epoll_event e;
static sigset_t signalset;
static struct sigaction sa;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED) {}

static void verify_sigmask(void)
{
	TEST(do_epoll_pwait(efd, &e, 1, -1, &signalset));

	if (TST_RET != 1) {
		tst_res(TFAIL, "do_epoll_pwait() returned %li, expected 1",
			TST_RET);
		return;
	}

	tst_res(TPASS, "do_epoll_pwait() with sigmask blocked signal");
}

static void verify_nonsigmask(void)
{
	TST_EXP_FAIL(do_epoll_pwait(efd, &e, 1, -1, NULL), EINTR,
		     "do_epoll_pwait() without sigmask");
}

static void (*testcase_list[])(void) = {verify_sigmask, verify_nonsigmask};

static void run(unsigned int n)
{
	char b;
	pid_t pid;

	if (!SAFE_FORK()) {
		pid = getppid();

		TST_PROCESS_STATE_WAIT(pid, 'S', 0);
		SAFE_KILL(pid, SIGUSR1);

		usleep(10000);
		SAFE_WRITE(SAFE_WRITE_ALL, sfd[1], "w", 1);
		exit(0);
	}

	testcase_list[n]();

	SAFE_READ(1, sfd[0], &b, 1);
	tst_reap_children();
}

static void setup(void)
{
	epoll_pwait_init();

	SAFE_SIGEMPTYSET(&signalset);
	SAFE_SIGADDSET(&signalset, SIGUSR1);

	sa.sa_flags = 0;
	sa.sa_handler = sighandler;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	SAFE_SOCKETPAIR(AF_UNIX, SOCK_STREAM, 0, sfd);

	efd = epoll_create(1);
	if (efd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create()");

	e.events = EPOLLIN;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd[0], &e))
		tst_brk(TBROK | TERRNO, "epoll_ctl(..., EPOLL_CTL_ADD, ...)");
}

static void cleanup(void)
{
	if (efd > 0)
		SAFE_CLOSE(efd);

	if (sfd[0] > 0)
		SAFE_CLOSE(sfd[0]);

	if (sfd[1] > 0)
		SAFE_CLOSE(sfd[1]);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test_variants = TEST_VARIANTS,
	.tcnt = ARRAY_SIZE(testcase_list),
};
