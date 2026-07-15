// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` rotates its ready list fairly.
 *
 * When more file descriptors are ready than the ``maxevents`` limit and the
 * descriptors are kept ready (never drained), the kernel has to rotate the
 * ready list so that all of them are reported in a round-robin fashion. Over
 * several full rotations every descriptor must therefore be reported the same
 * number of times, none may be starved and none may be favoured.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"

/* NREADY is deliberately not a multiple of MAXEVENTS. */
#define NREADY 13
#define MAXEVENTS 3
#define ROUNDS 4

static int epfd = -1;
static int ready_fds[NREADY][2];
static struct epoll_event *events;

static void setup(void)
{
	struct epoll_event ev = {.events = EPOLLIN};
	int i;

	for (i = 0; i < NREADY; i++) {
		ready_fds[i][0] = -1;
		ready_fds[i][1] = -1;
	}

	epfd = SAFE_EPOLL_CREATE1(0);

	for (i = 0; i < NREADY; i++) {
		SAFE_PIPE(ready_fds[i]);
		SAFE_WRITE(SAFE_WRITE_ALL, ready_fds[i][1], "x", 1);

		ev.data.u64 = i;
		SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, ready_fds[i][0], &ev);
	}
}

static void cleanup(void)
{
	int i;

	if (epfd != -1)
		SAFE_CLOSE(epfd);

	for (i = 0; i < NREADY; i++) {
		if (ready_fds[i][0] != -1) {
			SAFE_CLOSE(ready_fds[i][0]);
			SAFE_CLOSE(ready_fds[i][1]);
		}
	}
}

static void run(void)
{
	int count[NREADY] = {};
	int collected = 0, target = ROUNDS * NREADY;
	int i, ret;

	while (collected < target) {
		ret = SAFE_EPOLL_WAIT(epfd, events, MAXEVENTS, 0);

		if (ret != MAXEVENTS) {
			tst_res(TFAIL,
				"epoll_wait() returned %d events, expected %d",
				ret, MAXEVENTS);
			return;
		}

		for (i = 0; i < ret && collected < target; i++) {
			uint64_t idx = events[i].data.u64;

			if (idx >= NREADY) {
				tst_res(TFAIL,
					"epoll_wait() returned bogus index %llu",
					(unsigned long long)idx);
				return;
			}

			count[idx]++;
			collected++;
		}
	}

	for (i = 0; i < NREADY; i++) {
		if (count[i] != ROUNDS) {
			tst_res(TFAIL,
				"fd %d reported %d times, expected %d",
				i, count[i], ROUNDS);
			return;
		}
	}

	tst_res(TPASS,
		"epoll_wait() reported each of %d fds exactly %d times",
		NREADY, ROUNDS);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.bufs = (struct tst_buffers []) {
		{&events, .size = MAXEVENTS * sizeof(struct epoll_event)},
		{},
	},
};
