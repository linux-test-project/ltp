// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify the EPOLLEXCLUSIVE wakeup semantics of :manpage:`epoll_ctl(2)` and
 * :manpage:`epoll_wait(2)`.
 *
 * When a single target file descriptor is added with EPOLLEXCLUSIVE into
 * several epoll instances that each have a waiter blocked in
 * :manpage:`epoll_wait(2)`, one event on the target must wake exactly one
 * waiter. This is the thundering-herd avoidance introduced in Linux 4.5.
 *
 * The test writes into a pipe in a loop and checks that on each write exactly
 * one child process waiting on the fd is woken up.
 */

#include <sys/epoll.h>
#include <sys/mman.h>

#include "tst_test.h"
#include "tst_epoll.h"
#include "tst_atomic.h"
#include "lapi/epoll.h"

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
	struct epoll_event ev = {.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE};

	efd = SAFE_EPOLL_CREATE1(0);
	SAFE_EPOLL_CTL(efd, EPOLL_CTL_ADD, fds[0], &ev);

	TEST(epoll_wait(efd, &ev, 1, 2000));

	if (TST_RET == 1) {
		tst_atomic_add_return(1, woken);
		TST_CHECKPOINT_WAKE(0);
	} else {
		tst_res(TWARN | TTERRNO,
			"epoll_wait() returned %li expected 1", TST_RET);
	}

	SAFE_CLOSE(efd);
	exit(0);
}

static void run(void)
{
	int i, round, nwoken;
	char c;

	tst_atomic_store(0, woken);

	SAFE_PIPE(fds);

	for (i = 0; i < NWAITERS; i++) {
		pids[i] = SAFE_FORK();
		if (!pids[i])
			child();
	}

	for (i = 0; i < NWAITERS; i++)
		TST_PROCESS_STATE_WAIT(pids[i], 'S', 0);

	for (round = 0; round < NWAITERS; round++) {
		SAFE_WRITE(SAFE_WRITE_ALL, fds[1], "x", 1);

		TST_CHECKPOINT_WAIT(0);

		nwoken = tst_atomic_load(woken);
		if (nwoken != round + 1) {
			tst_res(TFAIL,
				"event %d woke %d waiters in total, expected %d",
				round + 1, nwoken, round + 1);
			break;
		}

		SAFE_READ(1, fds[0], &c, 1);
	}

	if (round == NWAITERS)
		tst_res(TPASS, "each event woke exactly one waiter");

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.min_kver = "4.5",
};
