// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`epoll_wait(2)` caps the number of reported events at
 * ``maxevents`` and eventually reports every ready file descriptor while
 * ignoring the registered descriptors that are not ready.
 *
 * When more file descriptors are ready than the ``maxevents`` limit passed to
 * :manpage:`epoll_wait(2)`, a single call must return at most ``maxevents``
 * events. The remaining ready descriptors have to be reported by subsequent
 * calls. Descriptors that are registered but have no data ready must never be
 * reported, so the test also registers a set of quiet descriptors and checks
 * that none of them show up in the results.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "tst_epoll.h"
#include "tst_minmax.h"

/* NREADY is deliberately not a multiple of MAXEVENTS. */
#define NREADY 13
#define NQUIET 5
#define MAXEVENTS 3

/*
 * Ready fds are registered with data.u64 in the range [0, NREADY), quiet fds
 * with data.u64 >= NREADY, so any reported quiet fd is trivially detected.
 */
#define QUIET_TAG NREADY

static int epfd = -1;
static int ready_fds[NREADY][2];
static int quiet_fds[NQUIET][2];
static struct epoll_event *events;

static void register_fds(int fds[][2], int n, uint64_t tag)
{
	struct epoll_event ev = {.events = EPOLLIN};
	int i;

	for (i = 0; i < n; i++) {
		SAFE_PIPE(fds[i]);

		ev.data.u64 = tag + i;
		SAFE_EPOLL_CTL(epfd, EPOLL_CTL_ADD, fds[i][0], &ev);
	}
}

static void close_fds(int fds[][2], int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (fds[i][0] != -1) {
			SAFE_CLOSE(fds[i][0]);
			SAFE_CLOSE(fds[i][1]);
		}
	}
}

static void init_fds(int fds[][2], int n)
{
	int i;

	for (i = 0; i < n; i++) {
		fds[i][0] = -1;
		fds[i][1] = -1;
	}
}

static void setup(void)
{
	init_fds(ready_fds, NREADY);
	init_fds(quiet_fds, NQUIET);

	epfd = SAFE_EPOLL_CREATE1(0);

	register_fds(ready_fds, NREADY, 0);
	register_fds(quiet_fds, NQUIET, QUIET_TAG);
}

static void cleanup(void)
{
	if (epfd != -1)
		SAFE_CLOSE(epfd);

	close_fds(ready_fds, NREADY);
	close_fds(quiet_fds, NQUIET);
}

static void run(void)
{
	int seen[NREADY] = {};
	int total = 0, calls = 0;
	int i, ret, remaining, exp_events;
	char c;

	for (i = 0; i < NREADY; i++)
		SAFE_WRITE(SAFE_WRITE_ALL, ready_fds[i][1], "x", 1);

	while (total < NREADY) {
		ret = SAFE_EPOLL_WAIT(epfd, events, MAXEVENTS, 0);
		calls++;

		remaining = NREADY - total;
		exp_events = MIN(remaining, MAXEVENTS);

		if (ret != exp_events) {
			tst_res(TFAIL,
				"epoll_wait() returned %d events, expected %d",
				ret, exp_events);
			return;
		}

		for (i = 0; i < ret; i++) {
			uint64_t idx = events[i].data.u64;

			if (idx >= NREADY) {
				tst_res(TFAIL,
					"epoll_wait() reported quiet fd (tag %llu)",
					(unsigned long long)idx);
				return;
			}

			if (seen[idx]) {
				tst_res(TFAIL,
					"epoll_wait() reported ready fd %llu twice",
					(unsigned long long)idx);
				return;
			}

			seen[idx] = 1;
			total++;

			SAFE_READ(1, ready_fds[idx][0], &c, 1);
		}
	}

	tst_res(TPASS, "epoll_wait() reported all %d ready fds in %d calls",
		NREADY, calls);
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
