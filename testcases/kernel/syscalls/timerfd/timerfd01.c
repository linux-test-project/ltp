// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  timerfd() test by Davide Libenzi (test app for timerfd)
 *  Copyright (C) 2007  Davide Libenzi
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 *  Description:
 *	Test timerfd with the flags:
 *		1) CLOCK_MONOTONIC
 *		2) CLOCK_REALTIME
 *
 * HISTORY
 *  28/05/2008 Initial contribution by Davide Libenzi <davidel@xmailserver.org>
 *  28/05/2008 Integrated to LTP by Subrata Modak <subrata@linux.vnet.ibm.com>
 */

#define _GNU_SOURCE
#include <poll.h>
#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_timerfd.h"

static struct tcase {
	int id;
	char const *name;
} tcases[] = {
	{CLOCK_MONOTONIC, "CLOCK MONOTONIC"},
	{CLOCK_REALTIME, "CLOCK REALTIME"},
};

static unsigned long long getustime(int clockid)
{
	struct timespec tp;

	if (clock_gettime((clockid_t) clockid, &tp)) {
		tst_res(TFAIL | TERRNO, "clock_gettime() failed");
		return 0;
	}

	return 1000000ULL * tp.tv_sec + tp.tv_nsec / 1000;
}

static void settime(int tfd, struct itimerspec *tmr, int tflags,
                    unsigned long long tvalue, int tinterval)
{
	tmr->it_value = tst_timespec_from_us(tvalue);
	tmr->it_interval = tst_timespec_from_us(tinterval);

	SAFE_TIMERFD_SETTIME(tfd, tflags, tmr, NULL);
}

static void waittmr(int tfd, unsigned int exp_ticks)
{
	uint64_t ticks;
	struct pollfd pfd;

	pfd.fd = tfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	if (poll(&pfd, 1, -1) < 0) {
		tst_res(TFAIL | TERRNO, "poll() failed");
		return;
	}
	if ((pfd.revents & POLLIN) == 0) {
		tst_res(TFAIL, "no ticks happened");
		return;
	}
	SAFE_READ(0, tfd, &ticks, sizeof(ticks));

	if (ticks != exp_ticks) {
		tst_res(TFAIL, "got %u tick(s) expected %u",
		        (unsigned int)ticks, exp_ticks);
	} else {
		tst_res(TPASS, "got %u tick(s)", exp_ticks);
	}
}

static void run(unsigned int n)
{
	int tfd;
	unsigned long long tnow;
	uint64_t uticks;
	struct itimerspec tmr;
	struct tcase *clks = &tcases[n];

	tst_res(TINFO, "testing %s", clks->name);

	tfd = SAFE_TIMERFD_CREATE(clks->id, 0);

	tst_res(TINFO, "relative timer (100 ms)");
	settime(tfd, &tmr, 0, 100 * 1000, 0);
	waittmr(tfd, 1);

	tst_res(TINFO, "absolute timer (100 ms)");
	tnow = getustime(clks->id);
	settime(tfd, &tmr, TFD_TIMER_ABSTIME, tnow + 100 * 1000, 0);
	waittmr(tfd, 1);

	tst_res(TINFO, "sequential timer (50 ms)");
	tnow = getustime(clks->id);
	settime(tfd, &tmr, TFD_TIMER_ABSTIME, tnow + 50 * 1000, 50 * 1000);

	memset(&tmr, 0, sizeof(tmr));
	if (timerfd_gettime(tfd, &tmr))
		tst_res(TFAIL | TERRNO, "timerfd_gettime() failed");


	if (tmr.it_value.tv_sec != 0 || tmr.it_value.tv_nsec > 50 * 1000000)
		tst_res(TFAIL, "Timer read back value not relative");
	else
		tst_res(TPASS, "Timer read back value is relative");

	usleep(160000);

	waittmr(tfd, 3);

	tst_res(TINFO, "testing with O_NONBLOCK");
	settime(tfd, &tmr, 0, 100 * 1000, 0);
	waittmr(tfd, 1);

	SAFE_FCNTL(tfd, F_SETFL, fcntl(tfd, F_GETFL, 0) | O_NONBLOCK);

	TEST(read(tfd, &uticks, sizeof(uticks)));
	if (TST_RET > 0)
		tst_res(TFAIL, "timer ticks not zero");
	else if (TST_ERR != EAGAIN)
		tst_res(TFAIL | TERRNO, "read should fail with EAGAIN got");
	else
		tst_res(TPASS | TERRNO, "read failed with");

	SAFE_CLOSE(tfd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "2.6.25",
};
