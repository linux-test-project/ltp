// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Red Hat, Inc., 2012.
 * Copyright (c) Linux Test Project, 2019
 *
 * Author:	Lingzhu Xiang <lxiang@redhat.com>
 *
 * Ported to new library:
 * 07/2019	Christian Amann <camann@suse.com>
 */

/*\
 * Regression test for hrtimer early expiration during and after leap seconds
 *
 * A bug in the hrtimer subsystem caused all TIMER_ABSTIME CLOCK_REALTIME
 * timers to expire one second early during leap second.
 * See http://lwn.net/Articles/504658/.
 *
 * This is a regression test for the bug.
 *
 * Note: running this test simultaneously with a timesync daemon
 * (e.g. systemd-timesyncd) can cause this test to fail.
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "tst_test.h"
#include "tst_safe_clocks.h"
#include "lapi/common_timers.h"

#define SECONDS_BEFORE_LEAP 2
#define SECONDS_AFTER_LEAP 2

static int errors;

static const char *strtime(const struct timespec *now)
{
	static char fmt[256], buf[256];

	if (snprintf(fmt, sizeof(fmt), "%%T.%09ld", now->tv_nsec) < 0) {
		buf[0] = '\0';
		return buf;
	}
	if (!strftime(buf, sizeof(buf), fmt, localtime(&now->tv_sec))) {
		buf[0] = '\0';
		return buf;
	}
	return buf;
}

static inline int in_order(struct timespec a, struct timespec b)
{
	if (a.tv_sec < b.tv_sec)
		return 1;
	if (a.tv_sec > b.tv_sec)
		return 0;
	if (a.tv_nsec > b.tv_nsec)
		return 0;
	return 1;
}

static void adjtimex_status(struct timex *tx, int status)
{
	const char *const msgs[6] = {
		"clock synchronized",
		"insert leap second",
		"delete leap second",
		"leap second in progress",
		"leap second has occurred",
		"clock not synchronized",
	};

	int ret;
	struct timespec now;

	tx->modes = ADJ_STATUS;
	tx->status = status;
	ret = adjtimex(tx);
	now.tv_sec = tx->time.tv_sec;
	now.tv_nsec = tx->time.tv_usec * 1000;

	if ((tx->status & status) != status)
		tst_brk(TBROK, "adjtimex status %d not set", status);
	else if (ret < 0)
		tst_brk(TBROK | TERRNO, "adjtimex");
	else if (ret < 6)
		tst_res(TINFO, "%s adjtimex: %s", strtime(&now), msgs[ret]);
	else
		tst_res(TINFO, "%s adjtimex: clock state %d",
			 strtime(&now), ret);
}

static void test_hrtimer_early_expiration(void)
{
	struct timespec now, target;
	int ret;

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &now);
	tst_res(TINFO, "now is     %s", strtime(&now));

	target = now;
	target.tv_sec++;
	tst_res(TINFO, "sleep until %s", strtime(&target));
	ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &target, NULL);
	if (ret < 0) {
		tst_res(TINFO | TERRNO, "clock_nanosleep");
		return;
	}

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &now);
	tst_res(TINFO, "now is     %s", strtime(&now));

	if (in_order(target, now)) {
		tst_res(TINFO, "hrtimer early expiration is not detected.");
	} else {
		tst_res(TFAIL, "hrtimer early expiration is detected.");
		errors++;
	}
}

static void run_leapsec(void)
{
	const struct timespec sleeptime = { 0, NSEC_PER_SEC / 2 };
	struct timespec now, leap, start;
	struct timex tx;

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &now);
	start = now;
	tst_res(TINFO, "test start at %s", strtime(&now));

	test_hrtimer_early_expiration();

	/* calculate the next leap second */
	now.tv_sec += 86400 - now.tv_sec % 86400;
	now.tv_nsec = 0;
	leap = now;
	tst_res(TINFO, "scheduling leap second %s", strtime(&leap));

	/* start before the leap second */
	now.tv_sec -= SECONDS_BEFORE_LEAP;
	SAFE_CLOCK_SETTIME(CLOCK_REALTIME, &now);

	tst_res(TINFO, "setting time to        %s", strtime(&now));

	/* reset NTP time state */
	adjtimex_status(&tx, STA_PLL);
	adjtimex_status(&tx, 0);

	/* set the leap second insert flag */
	adjtimex_status(&tx, STA_INS);

	/* reliably sleep until after the leap second */
	while (tx.time.tv_sec < leap.tv_sec + SECONDS_AFTER_LEAP) {
		adjtimex_status(&tx, tx.status);
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL);
	}

	test_hrtimer_early_expiration();

	adjtimex_status(&tx, STA_PLL);
	adjtimex_status(&tx, 0);

	/* recover from timer expiring state and restore time */
	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &now);
	start.tv_sec += now.tv_sec - (leap.tv_sec - SECONDS_BEFORE_LEAP);
	start.tv_nsec += now.tv_nsec;
	start.tv_sec += start.tv_nsec / NSEC_PER_SEC;
	start.tv_nsec = start.tv_nsec % NSEC_PER_SEC;
	tst_res(TINFO, "restoring time to %s", strtime(&start));
	/* calls clock_was_set() in kernel to revert inconsistency */
	SAFE_CLOCK_SETTIME(CLOCK_REALTIME, &start);

	test_hrtimer_early_expiration();

	if (!errors)
		tst_res(TPASS, "No errors were reported during this test!");
	else
		tst_res(TFAIL, "Got %d errors during this test!", errors);

}

static void setup(void)
{
	errors = 0;
}

static void cleanup(void)
{
	struct timespec now;

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &now);
	/* Calls clock_was_set() in the kernel to revert inconsistencies.
	 * The only possible error EPERM doesn't matter here. */
	SAFE_CLOCK_SETTIME(CLOCK_REALTIME, &now);
}

static struct tst_test test = {
	.timeout = 40,
	.test_all = run_leapsec,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
};
