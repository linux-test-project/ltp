// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * This program opens the PID file descriptor of the child process created with
 * fork(). It then uses poll to monitor the file descriptor for process exit, as
 * indicated by an EPOLLIN event.
 */
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/pidfd_open.h"

static void run(void)
{
	struct pollfd pollfd;
	int fd, ready;
	pid_t pid;

	pid = SAFE_FORK();

	if (!pid) {
		TST_CHECKPOINT_WAIT(0);
		exit(EXIT_SUCCESS);
	}

	TEST(pidfd_open(pid, 0));

	fd = TST_RET;
	if (fd == -1)
		tst_brk(TFAIL | TTERRNO, "pidfd_open() failed");

	TST_CHECKPOINT_WAKE(0);

	pollfd.fd = fd;
	pollfd.events = POLLIN;

	ready = poll(&pollfd, 1, -1);

	SAFE_CLOSE(fd);
	SAFE_WAITPID(pid, NULL, 0);

	if (ready != 1)
		tst_res(TFAIL, "poll() returned %d", ready);
	else
		tst_res(TPASS, "pidfd_open() passed");
}

static struct tst_test test = {
	.min_kver = "5.3",
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
