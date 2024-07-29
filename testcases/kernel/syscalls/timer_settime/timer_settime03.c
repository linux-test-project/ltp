// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE 2018-12896
 *
 * Check for possible overflow of posix timer overrun counter. Create
 * a CLOCK_REALTIME timer, set extremely low timer interval and expiration
 * value just right to cause overrun overflow into negative values, start
 * the timer with TIMER_ABSTIME flag to cause overruns immediately. Then just
 * check the overrun counter in the timer signal handler. On a patched system,
 * the value returned by timer_getoverrun() should be capped at INT_MAX and
 * not allowed to overflow into negative range. Bug fixed in:
 *
 *  commit 78c9c4dfbf8c04883941445a195276bb4bb92c76
 *  Author: Thomas Gleixner <tglx@linutronix.de>
 *  Date:   Tue Jun 26 15:21:32 2018 +0200
 *
 *  posix-timers: Sanitize overrun handling
 */

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#include "tst_test.h"
#include "tst_safe_clocks.h"

static timer_t timer;
static volatile int handler_called, overrun, saved_errno;

static struct timespec realtime_resolution;

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
	struct itimerspec spec;

	/*
	 * Signal handler will be called twice in total because kernel will
	 * schedule another pending signal before the timer gets disabled.
	 */
	if (handler_called)
		return;

	errno = 0;
	overrun = timer_getoverrun(timer);
	saved_errno = errno;
	memset(&spec, 0, sizeof(struct itimerspec));
	SAFE_TIMER_SETTIME(timer, 0, &spec, NULL);
	handler_called = 1;
}

static void setup(void)
{
	struct sigevent sev;

	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGUSR1;

	SAFE_SIGNAL(SIGUSR1, sighandler);
	SAFE_TIMER_CREATE(CLOCK_REALTIME, &sev, &timer);

	SAFE_CLOCK_GETRES(CLOCK_REALTIME, &realtime_resolution);

	tst_res(TINFO, "CLOCK_REALTIME resolution %lins",
		(long)realtime_resolution.tv_nsec);
}

static void run(void)
{
	int handler_delay = INT_MAX / 7;
	long nsec;
	struct itimerspec spec;

	handler_called = 0;
	memset(&spec, 0, sizeof(struct itimerspec));
	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &spec.it_value);
	nsec = (handler_delay % 100000000) * 10L;

	if (nsec > spec.it_value.tv_nsec) {
		spec.it_value.tv_sec -= 1;
		spec.it_value.tv_nsec += 1000000000;
	}

	/* spec.it_value = now - 1.4 * max overrun value */
	/* IOW, overflow will land right in the middle of negative range */
	spec.it_value.tv_sec -= (handler_delay / 100000000) * realtime_resolution.tv_nsec;
	spec.it_value.tv_nsec -= nsec;
	spec.it_interval.tv_nsec = realtime_resolution.tv_nsec;

	SAFE_TIMER_SETTIME(timer, TIMER_ABSTIME, &spec, NULL);
	while (!handler_called)
		;

	errno = saved_errno;

	if (overrun == -1)
		tst_brk(TBROK | TERRNO, "Error reading timer overrun count");

	if (overrun == INT_MAX) {
		tst_res(TPASS, "Timer overrun count is capped");
		return;
	}

	if (overrun < 0) {
		tst_res(TFAIL, "Timer overrun counter overflow");
		return;
	}

	tst_res(TFAIL, "Timer overrun counter is wrong: %d; expected %d or "
		"negative number", overrun, INT_MAX);
}

static void cleanup(void)
{
	SAFE_TIMER_DELETE(timer);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "78c9c4dfbf8c"},
		{"CVE", "2018-12896"},
		{}
	}
};
