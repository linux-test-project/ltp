// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` fails with EINTR when a signal arrives
 * while waiting for events.
 *
 * [Algorithm]
 *
 * - Create an epoll instance
 * - Fork a child that waits in the :manpage:`epoll_wait(2)`, parent sends
 *   SIGUSR1 to interrupt it.
 * - Verify that :manpage:`epoll_wait(2)` returns -1 with errno set to EINTR.
 */

#include <stdlib.h>
#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"

static int efd = -1;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	static struct sigaction sa = {
		.sa_handler = sighandler,
	};

	SAFE_SIGEMPTYSET(&sa.sa_mask);
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	efd = SAFE_EPOLL_CREATE1(0);
}

static void run(void)
{
	pid_t pid = SAFE_FORK();

	if (!pid) {
		struct epoll_event ev;

		TST_EXP_FAIL2(epoll_wait(efd, &ev, 1, -1), EINTR,
			     "epoll_wait() interrupted by signal");
		exit(0);
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_KILL(pid, SIGUSR1);
}

static void cleanup(void)
{
	if (efd != -1)
		SAFE_CLOSE(efd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
