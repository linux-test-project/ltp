/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

/*
 * Description:
 *  Basic test for epoll_pwait(2).
 *  1) epoll_pwait(2) with sigmask argument allows the caller to
 *     safely wait until either a file descriptor becomes ready
 *     or the timeout expires.
 *  2) epoll_pwait(2) with NULL sigmask argument fails if
 *     interrupted by a signal handler, epoll_pwait(2) should
 *     return -1 and set errno to EINTR.
 */

#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "epoll_pwait.h"
#include "safe_macros.h"

char *TCID = "epoll_pwait01";
int TST_TOTAL = 2;

static int epfd, fds[2];
static sigset_t signalset;
static struct epoll_event epevs;
static struct sigaction sa;

static void setup(void);
static void verify_sigmask(void);
static void verify_nonsigmask(void);
static void sighandler(int sig LTP_ATTRIBUTE_UNUSED);
static void do_test(sigset_t *);
static void do_child(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		do_test(&signalset);
		do_test(NULL);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 19)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			 "that are 2.6.19 or higher");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if (sigemptyset(&signalset) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "sigemptyset() failed");

	if (sigaddset(&signalset, SIGUSR1) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "sigaddset() failed");

	sa.sa_flags = 0;
	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "sigemptyset() failed");

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "sigaction() failed");

	SAFE_PIPE(NULL, fds);

	epfd = epoll_create(1);
	if (epfd == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to create epoll instance");
	}

	epevs.events = EPOLLIN;
	epevs.data.fd = fds[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to register epoll target");
	}
}

static void verify_sigmask(void)
{
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "epoll_pwait() failed");
		return;
	}

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL, "epoll_pwait() returned %li, expected 0",
			 TEST_RETURN);
		return;
	}

	tst_resm(TPASS, "epoll_pwait(sigmask) blocked signal");
}

static void verify_nonsigmask(void)
{
	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "epoll_wait() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == EINTR) {
		tst_resm(TPASS | TTERRNO, "epoll_wait() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO, "epoll_wait() failed unexpectedly, "
				 "expected EINTR");
	}
}

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{

}

static void do_test(sigset_t *sigmask)
{
	pid_t cpid;

	cpid = tst_fork();
	if (cpid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");

	if (cpid == 0)
		do_child();

	TEST(epoll_pwait(epfd, &epevs, 1, 100, sigmask));

	if (sigmask != NULL)
		verify_sigmask();
	else
		verify_nonsigmask();

	tst_record_childstatus(cleanup, cpid);
}

static void do_child(void)
{
	if (tst_process_state_wait2(getppid(), 'S') != 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to wait for parent process's state");
	}

	SAFE_KILL(cleanup, getppid(), SIGUSR1);

	cleanup();
	tst_exit();
}

static void cleanup(void)
{
	if (epfd > 0 && close(epfd))
		tst_resm(TWARN | TERRNO, "failed to close epfd");

	if (close(fds[0]))
		tst_resm(TWARN | TERRNO, "close(fds[0]) failed");

	if (close(fds[1]))
		tst_resm(TWARN | TERRNO, "close(fds[1]) failed");
}
