// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic test for epoll_wait. Check that epoll_wait works for EPOLLOUT and
 * EPOLLIN events on an epoll instance and that struct epoll_event is set
 * correctly.
 */

#include <sys/epoll.h>
#include <poll.h>
#include <string.h>
#include <errno.h>

#include "tst_test.h"

static int write_size, epfd, fds[2];

static int get_writesize(void)
{
	int nfd, write_size = 0;
	char buf[4096];

	struct pollfd pfd[] = {
		{.fd = fds[1], .events = POLLOUT},
	};

	memset(buf, 'a', sizeof(buf));

	do {
		write_size += SAFE_WRITE(0, fds[1], buf, sizeof(buf));
		nfd = poll(pfd, 1, 1);
		if (nfd == -1)
			tst_brk(TBROK | TERRNO, "poll() failed");
	} while (nfd > 0);

	char read_buf[write_size];

	SAFE_READ(1, fds[0], read_buf, sizeof(read_buf));

	tst_res(TINFO, "Pipe buffer size is %i bytes", write_size);

	return write_size;
}

static void setup(void)
{
	static struct epoll_event epevs[2] = {
		{.events = EPOLLIN},
		{.events = EPOLLOUT},
	};

	SAFE_PIPE(fds);

	epevs[0].data.fd = fds[0];
	epevs[1].data.fd = fds[1];

	write_size = get_writesize();

	epfd = epoll_create(3);
	if (epfd == -1)
		tst_brk(TBROK | TERRNO, "epoll_create() failed");


	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs[0]) ||
	    epoll_ctl(epfd, EPOLL_CTL_ADD, fds[1], &epevs[1])) {
		tst_brk(TBROK | TERRNO, "epoll_ctl() failed");
	}
}

static int has_event(struct epoll_event *epevs, int epevs_len,
		     int fd, uint32_t events)
{
	int i;

	for (i = 0; i < epevs_len; i++) {
		if ((epevs[i].data.fd == fd) && (epevs[i].events == events))
			return 1;
	}

	return 0;
}

static void dump_epevs(struct epoll_event *epevs, int epevs_len)
{
	int i;

	for (i = 0; i < epevs_len; i++) {
		tst_res(TINFO, "epevs[%d]: epoll.data.fd %d, epoll.events %x",
			i, epevs[i].data.fd, epevs[i].events);
	}
}

static void verify_epollout(void)
{
	struct epoll_event ret_evs = {.events = 0, .data.fd = 0};

	TEST(epoll_wait(epfd, &ret_evs, 1, -1));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "epoll_wait() epollout failed");
		return;
	}

	if (TST_RET != 1) {
		tst_res(TFAIL, "epoll_wait() returned %li, expected 1",
			TST_RET);
		return;
	}

	if (ret_evs.data.fd != fds[1]) {
		tst_res(TFAIL, "epoll.data.fd %i, expected %i",
			ret_evs.data.fd, fds[1]);
		return;
	}

	if (ret_evs.events != EPOLLOUT) {
		tst_res(TFAIL, "epoll.events %x, expected EPOLLOUT %x",
			ret_evs.events, EPOLLOUT);
		return;
	}

	tst_res(TPASS, "epoll_wait() epollout");
}

static void verify_epollin(void)
{
	char write_buf[write_size];
	char read_buf[sizeof(write_buf)];
	struct epoll_event ret_evs = {.events = 0, .data.fd = 0};

	memset(write_buf, 'a', sizeof(write_buf));

	SAFE_WRITE(1, fds[1], write_buf, sizeof(write_buf));

	TEST(epoll_wait(epfd, &ret_evs, 1, -1));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "epoll_wait() epollin failed");
		goto end;
	}

	if (TST_RET != 1) {
		tst_res(TFAIL, "epoll_wait() returned %li, expected 1",
			TST_RET);
		goto end;
	}

	if (ret_evs.data.fd != fds[0]) {
		tst_res(TFAIL, "epoll.data.fd %i, expected %i",
			ret_evs.data.fd, fds[0]);
		goto end;
	}

	if (ret_evs.events != EPOLLIN) {
		tst_res(TFAIL, "epoll.events %x, expected EPOLLIN %x",
			ret_evs.events, EPOLLIN);
		goto end;
	}

	tst_res(TPASS, "epoll_wait() epollin");

end:
	SAFE_READ(1, fds[0], read_buf, sizeof(write_buf));
}

static void verify_epollio(void)
{
	char write_buf[] = "Testing";
	char read_buf[sizeof(write_buf)];
	uint32_t events = EPOLLIN | EPOLLOUT;
	struct epoll_event ret_evs[2];

	SAFE_WRITE(1, fds[1], write_buf, sizeof(write_buf));

	while (events) {
		int events_matched = 0;

		memset(ret_evs, 0, sizeof(ret_evs));
		TEST(epoll_wait(epfd, ret_evs, 2, -1));

		if (TST_RET <= 0) {
			tst_res(TFAIL | TTERRNO, "epoll_wait() returned %li",
				TST_RET);
			goto end;
		}

		if ((events & EPOLLIN) &&
		    has_event(ret_evs, 2, fds[0], EPOLLIN)) {
			events_matched++;
			events &= ~EPOLLIN;
		}

		if ((events & EPOLLOUT) &&
		    has_event(ret_evs, 2, fds[1], EPOLLOUT)) {
			events_matched++;
			events &= ~EPOLLOUT;
		}

		if (TST_RET != events_matched) {
			tst_res(TFAIL,
				"epoll_wait() returned unexpected events");
			dump_epevs(ret_evs, 2);
			goto end;
		}
	}

	tst_res(TPASS, "epoll_wait() epollio");

end:
	SAFE_READ(1, fds[0], read_buf, sizeof(write_buf));
}

static void cleanup(void)
{
	if (epfd > 0)
		SAFE_CLOSE(epfd);

	if (fds[0]) {
		SAFE_CLOSE(fds[0]);
		SAFE_CLOSE(fds[1]);
	}
}

static void (*testcase_list[])(void) = {
	verify_epollout, verify_epollin, verify_epollio
};

static void do_test(unsigned int n)
{
	testcase_list[n]();
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(testcase_list),
};
