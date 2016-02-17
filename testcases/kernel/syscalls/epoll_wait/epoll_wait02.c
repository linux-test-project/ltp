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
 *  Check that epoll_wait(2) timeouts correctly.
 */

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "epoll_wait02";
int TST_TOTAL = 1;

static int epfd, fds[2];
static char *opt_sleep_ms;
static struct epoll_event epevs[1] = {
	{.events = EPOLLIN}
};

static option_t opts[] = {
	{"s:", NULL, &opt_sleep_ms},
	{NULL, NULL, NULL}
};

static void setup(void);
static void cleanup(void);
static void help(void);

int main(int ac, char **av)
{
	int lc, threshold;
	long long elapsed_ms, sleep_ms = 100;

	tst_parse_opts(ac, av, opts, help);

	if (opt_sleep_ms) {
		sleep_ms = atoll(opt_sleep_ms);

		if (sleep_ms == 0) {
			tst_brkm(TBROK, NULL,
				 "Invalid timeout '%s'", opt_sleep_ms);
		}
	}

	threshold = sleep_ms / 100 + 10;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		tst_timer_start(CLOCK_MONOTONIC);
		TEST(epoll_wait(epfd, epevs, 1, sleep_ms));
		tst_timer_stop();

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "epoll_wait() failed");
			continue;
		}

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "epoll_wait() returned %li, expected 0",
				 TEST_RETURN);
			continue;
		}

		elapsed_ms = tst_timer_elapsed_ms();

		if (elapsed_ms < sleep_ms) {
			tst_resm(TFAIL, "epoll_wait() woken up too early %llims, "
				 "expected %llims", elapsed_ms, sleep_ms);
			continue;
		}

		if (elapsed_ms - sleep_ms > threshold) {
			tst_resm(TFAIL, "epoll_wait() slept too long %llims, "
				 "expected %llims, threshold %i",
				 elapsed_ms, sleep_ms, threshold);
			continue;
		}

		tst_resm(TPASS, "epoll_wait() slept %llims, expected %llims, "
			 "threshold %i", elapsed_ms, sleep_ms, threshold);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_timer_check(CLOCK_MONOTONIC);

	SAFE_PIPE(NULL, fds);

	epfd = epoll_create(1);
	if (epfd == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to create epoll instance");
	}

	epevs[0].data.fd = fds[0];

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &epevs[0])) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to register epoll target");
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

static void help(void)
{
	printf("  -s      epoll_wait() timeout length in ms\n");
}
