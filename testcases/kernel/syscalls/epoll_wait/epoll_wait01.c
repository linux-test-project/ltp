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
 *  Basic test for epoll_wait(2).
 *  Check that epoll_wait(2) works for EPOLLOUT and EPOLLIN events
 *  on a epoll instance and that struct epoll_event is set correctly.
 */

#include <sys/epoll.h>
#include <poll.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "epoll_wait01";
int TST_TOTAL = 3;

static int write_size, epfd, fds[2], fds2[2];
static struct epoll_event epevs[3] = {
	{.events = EPOLLIN},
	{.events = EPOLLOUT},
	{.events = EPOLLIN}
};

static void setup(void);
static int get_writesize(void);
static void verify_epollout(void);
static void verify_epollin(void);
static void verify_epollio(void);
static int has_event(struct epoll_event *, int, int, uint32_t);
static void dump_epevs(struct epoll_event *, int);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		verify_epollout();
		verify_epollin();
		verify_epollio();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	SAFE_PIPE(NULL, fds);
	SAFE_PIPE(cleanup, fds2);

	write_size = get_writesize();

	epfd = epoll_create(3);
	if (epfd == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to create epoll instance");
	}

	epevs[0].data.fd = fds[0];
	epevs[1].data.fd = fds[1];
	epevs[2].data.fd = fds2[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs[0]) ||
	    epoll_ctl(epfd, EPOLL_CTL_ADD, fds[1], &epevs[1]) ||
	    epoll_ctl(epfd, EPOLL_CTL_ADD, fds2[0], &epevs[2])) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to register epoll target");
	}
}

static int get_writesize(void)
{
	int nfd, write_size = 0;
	char buf[4096];

	struct pollfd pfd[] = {
		{.fd = fds[1], .events = POLLOUT},
	};

	memset(buf, 'a', sizeof(buf));

	do {
		write_size += SAFE_WRITE(cleanup, 0, fds[1], buf, sizeof(buf));
		nfd = poll(pfd, 1, 1);
		if (nfd == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "poll failed");
	} while (nfd > 0);

	char read_buf[write_size];

	SAFE_READ(cleanup, 1, fds[0], read_buf, sizeof(read_buf));

	tst_resm(TINFO, "Pipe buffer size is %i bytes", write_size);

	return write_size;
}

static void verify_epollout(void)
{
	TEST(epoll_wait(epfd, epevs, 3, -1));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "epoll_wait() epollout failed");
		return;
	}

	if (TEST_RETURN != 1) {
		tst_resm(TFAIL, "epoll_wait() returned %li, expected 1",
			 TEST_RETURN);
		return;
	}

	if (epevs[0].data.fd != fds[1]) {
		tst_resm(TFAIL, "epoll.data.fd %i, expected %i",
			 epevs[0].data.fd, fds[1]);
		return;
	}

	if (epevs[0].events != EPOLLOUT) {
		tst_resm(TFAIL, "epoll.events %x, expected EPOLLOUT %x",
			 epevs[0].events, EPOLLOUT);
		return;
	}

	tst_resm(TPASS, "epoll_wait() epollout");
}

static void verify_epollin(void)
{
	char write_buf[write_size];
	char read_buf[sizeof(write_buf)];

	memset(write_buf, 'a', sizeof(write_buf));

	SAFE_WRITE(cleanup, 1, fds[1], write_buf, sizeof(write_buf));

	TEST(epoll_wait(epfd, epevs, 3, -1));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "epoll_wait() epollin failed");
		goto end;
	}

	if (TEST_RETURN != 1) {
		tst_resm(TFAIL, "epoll_wait() returned %li, expected 1",
			 TEST_RETURN);
		goto end;
	}

	if (epevs[0].data.fd != fds[0]) {
		tst_resm(TFAIL, "epoll.data.fd %i, expected %i",
			 epevs[0].data.fd, fds[0]);
		goto end;
	}

	if (epevs[0].events != EPOLLIN) {
		tst_resm(TFAIL, "epoll.events %x, expected EPOLLIN %x",
			 epevs[0].events, EPOLLIN);
		goto end;
	}

	tst_resm(TPASS, "epoll_wait() epollin");

end:
	SAFE_READ(cleanup, 1, fds[0], read_buf, sizeof(write_buf));
}

static void verify_epollio(void)
{
	char write_buf[] = "Testing";
	char read_buf[sizeof(write_buf)];

	SAFE_WRITE(cleanup, 1, fds[1], write_buf, sizeof(write_buf));

	TEST(epoll_wait(epfd, epevs, 3, -1));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "epoll_wait() epollio failed");
		goto end;
	}

	if (TEST_RETURN != 2) {
		tst_resm(TFAIL, "epoll_wait() returned %li, expected 2",
			 TEST_RETURN);
		goto end;
	}

	if (!has_event(epevs, 2, fds[0], EPOLLIN)) {
		dump_epevs(epevs, 2);
		tst_resm(TFAIL, "epoll_wait() expected %d and EPOLLIN %x",
			 fds[0], EPOLLIN);
		goto end;
	}

	if (!has_event(epevs, 2, fds[1], EPOLLOUT)) {
		dump_epevs(epevs, 2);
		tst_resm(TFAIL, "epoll_wait() expected %d and EPOLLOUT %x",
			 fds[1], EPOLLOUT);
		goto end;
	}

	tst_resm(TPASS, "epoll_wait() epollio");

end:
	SAFE_READ(cleanup, 1, fds[0], read_buf, sizeof(write_buf));
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
		tst_resm(TINFO, "epevs[%d]: epoll.data.fd %d, epoll.events %x",
			 i, epevs[i].data.fd, epevs[i].events);
	}
}

static void cleanup(void)
{
	if (epfd > 0 && close(epfd))
		tst_resm(TWARN | TERRNO, "failed to close epfd");

	if (close(fds[0]))
		tst_resm(TWARN | TERRNO, "failed to close fds[0]");

	if (close(fds[1]))
		tst_resm(TWARN | TERRNO, "failed to close fds[1]");

	if (fds2[0] > 0 && close(fds2[0]))
		tst_resm(TWARN | TERRNO, "failed to close fds2[0]");

	if (fds2[1] > 0 && close(fds2[1]))
		tst_resm(TWARN | TERRNO, "failed to close fds2[1]");
}
