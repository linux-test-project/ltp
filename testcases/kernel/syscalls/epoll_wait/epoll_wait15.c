// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that a single event on a file descriptor monitored by several epoll
 * instances wakes all of the waiters blocked in :manpage:`epoll_wait(2)`.
 *
 * This is the default (non-EPOLLEXCLUSIVE) behavior and serves as the contrast
 * to the thundering-herd avoidance tested in epoll_wait16.
 */

#include <sys/epoll.h>
#include <sys/mman.h>

#include "tst_test.h"
#include "tst_epoll.h"
#include "tst_atomic.h"

#define NWAITERS 8

static int fds[2] = {-1, -1};
static pid_t pids[NWAITERS];
static tst_atomic_t *woken;

static void setup(void)
{
	woken = SAFE_MMAP(NULL, sizeof(*woken), PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (fds[0] != -1) {
		SAFE_CLOSE(fds[0]);
		SAFE_CLOSE(fds[1]);
	}

	if (woken)
		SAFE_MUNMAP(woken, sizeof(*woken));
}

static void child(void)
{
	int efd;
	struct epoll_event ev = {.events = EPOLLIN | EPOLLET};

	efd = SAFE_EPOLL_CREATE1(0);
	SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, fds[0], &ev);

	TST_CHECKPOINT_WAKE(0);

	TEST(epoll_wait(efd, &ev, 1, 2000));

	if (TST_RET == 1) {
		tst_atomic_add_return(1, woken);
	} else {
		tst_res(TWARN | TTERRNO,
			"epoll_wait() returned %li expected 1", TST_RET);
	}

	SAFE_CLOSE(efd);
	exit(0);
}

static void run(void)
{
	int i, nwoken;

	tst_atomic_store(0, woken);

	SAFE_PIPE(fds);

	for (i = 0; i < NWAITERS; i++) {
		pids[i] = SAFE_FORK();
		if (!pids[i])
			child();

		TST_CHECKPOINT_WAIT(0);
	}

	for (i = 0; i < NWAITERS; i++)
		TST_PROCESS_STATE_WAIT(pids[i], 'S', 0);

	SAFE_WRITE(SAFE_WRITE_ALL, fds[1], "x", 1);

	tst_reap_children();

	nwoken = tst_atomic_load(woken);

	if (nwoken == NWAITERS)
		tst_res(TPASS, "single event woke all %d waiters", NWAITERS);
	else
		tst_res(TFAIL, "single event woke %d of %d waiters",
			nwoken, NWAITERS);

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
