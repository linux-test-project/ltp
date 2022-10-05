// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Check the basic functionality of the epoll_ctl:
 *
 * - When epoll_ctl succeeds to register fd on the epoll instance and associates
 * event with fd, epoll_wait will get registered fd and event correctly.
 * - When epoll_ctl succeeds to change event which is related to fd, epoll_wait
 * will get changed event correctly.
 * - When epoll_ctl succeeds to deregister fd from the epoll instance epoll_wait
 * won't get deregistered fd and event.
 */

#include <poll.h>
#include <sys/epoll.h>

#include "tst_test.h"

static int epfd;
static int fd[2];

static struct epoll_event events[3] = {
	{.events = EPOLLIN},
	{.events = EPOLLOUT},
	{.events = EPOLLIN}
};

static void setup(void)
{
	epfd = epoll_create(2);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "fail to create epoll instance");

	SAFE_PIPE(fd);

	events[0].data.fd = fd[0];
	events[1].data.fd = fd[1];
	events[2].data.fd = fd[1];
}

static void cleanup(void)
{
	if (epfd > 0)
		SAFE_CLOSE(epfd);

	if (fd[0] > 0)
		SAFE_CLOSE(fd[0]);

	if (fd[1] > 0)
		SAFE_CLOSE(fd[1]);
}

static int has_event(struct epoll_event *epvs, int len,
	int fd, unsigned int events)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((epvs[i].data.fd == fd) && (epvs[i].events == events))
			return 1;
	}

	return 0;
}

static void check_epoll_ctl(int opt, int exp_num)
{
	int res;
	unsigned int events;
	char write_buf[] = "test";
	char read_buf[sizeof(write_buf)];
	struct epoll_event res_evs[2];

	events = EPOLLIN;
	if (exp_num == 2)
		events |= EPOLLOUT;

	SAFE_WRITE(SAFE_WRITE_ALL, fd[1], write_buf, sizeof(write_buf));

	while (events) {
		int events_matched = 0;

		memset(res_evs, 0, sizeof(res_evs));
		res = epoll_wait(epfd, res_evs, 2, -1);

		if (res <= 0) {
			tst_res(TFAIL | TERRNO, "epoll_wait() returned %i",
				res);
			goto end;
		}

		if ((events & EPOLLIN) &&
		    has_event(res_evs, 2, fd[0], EPOLLIN)) {
			events_matched++;
			events &= ~EPOLLIN;
		}

		if ((events & EPOLLOUT) &&
		    has_event(res_evs, 2, fd[1], EPOLLOUT)) {
			events_matched++;
			events &= ~EPOLLOUT;
		}

		if (res != events_matched) {
			tst_res(TFAIL,
				"epoll_wait() returned unexpected events");
			goto end;
		}
	}

	tst_res(TPASS, "epoll_ctl() succeeds with op %i", opt);

end:
	SAFE_READ(1, fd[0], read_buf, sizeof(write_buf));
}

static void opera_epoll_ctl(int opt, int fd, struct epoll_event *epvs)
{
	TEST(epoll_ctl(epfd, opt, fd, epvs));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "epoll_ctl() fails with op %i", opt);
}

static void verify_epoll_ctl(void)
{
	opera_epoll_ctl(EPOLL_CTL_ADD, fd[0], &events[0]);
	opera_epoll_ctl(EPOLL_CTL_ADD, fd[1], &events[2]);
	check_epoll_ctl(EPOLL_CTL_ADD, 1);
	opera_epoll_ctl(EPOLL_CTL_MOD, fd[1], &events[1]);
	check_epoll_ctl(EPOLL_CTL_MOD, 2);
	opera_epoll_ctl(EPOLL_CTL_DEL, fd[1], &events[1]);
	check_epoll_ctl(EPOLL_CTL_DEL, 1);
	opera_epoll_ctl(EPOLL_CTL_DEL, fd[0], &events[0]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_epoll_ctl,
};
