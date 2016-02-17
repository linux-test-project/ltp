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
 *  1) epoll_wait(2) fails if epfd is not a valid file descriptor
 *  2) epoll_wait(2) fails if epfd is not an epoll file descriptor
 *  3) epoll_wait(2) fails if maxevents is less than zero
 *  4) epoll_wait(2) fails if maxevents is equal to zero
 *  5) epoll_wait(2) fails if the memory area pointed to by events
 *     is not accessible with write permissions.
 *
 * Expected Result:
 *  1) epoll_wait(2) should return -1 and set errno to EBADF
 *  2) epoll_wait(2) should return -1 and set errno to EINVAL
 *  3) epoll_wait(2) should return -1 and set errno to EINVAL
 *  4) epoll_wait(2) should return -1 and set errno to EINVAL
 *  5) epoll_wait(2) should return -1 and set errno to EFAULT
 */

#include <sys/epoll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

static int page_size, fds[2], epfd, inv_epfd, bad_epfd = -1;

static struct epoll_event epevs[1] = {
	{.events = EPOLLOUT}
};

static struct epoll_event *ev_rdwr = epevs;
static struct epoll_event *ev_rdonly;

static struct test_case_t {
	int *epfd;
	struct epoll_event **ev;
	int maxevents;
	int exp_errno;
} tc[] = {
	/* test1 */
	{&bad_epfd, &ev_rdwr, 1, EBADF},
	/* test2 */
	{&inv_epfd, &ev_rdwr, 1, EINVAL},
	/* test3 */
	{&epfd, &ev_rdwr, -1, EINVAL},
	/* test4 */
	{&epfd, &ev_rdwr, 0, EINVAL},
	/* test5 */
	{&epfd, &ev_rdonly, 1, EFAULT}
};

char *TCID = "epoll_wait03";
int TST_TOTAL = ARRAY_SIZE(tc);

static void setup(void);
static void verify_epoll_wait(struct test_case_t *tc);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify_epoll_wait(&tc[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	page_size = getpagesize();

	ev_rdonly = SAFE_MMAP(NULL, NULL, page_size, PROT_READ,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	SAFE_PIPE(NULL, fds);

	epfd = epoll_create(1);
	if (epfd == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to create epoll instance");
	}

	epevs[0].data.fd = fds[1];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[1], &epevs[0])) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to register epoll target");
	}
}

static void verify_epoll_wait(struct test_case_t *tc)
{
	TEST(epoll_wait(*(tc->epfd), *(tc->ev), tc->maxevents, -1));
	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "epoll_wait() succeed unexpectedly");
	} else {
		if (tc->exp_errno == TEST_ERRNO) {
			tst_resm(TPASS | TTERRNO,
				 "epoll_wait() fails as expected");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "epoll_wait() fails unexpectedly, "
				 "expected %d: %s", tc->exp_errno,
				 tst_strerrno(tc->exp_errno));
		}
	}
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
