/*
 * Regression test for hrtimer early expiration during and after leap seconds
 *
 * A bug in the hrtimer subsystem caused all TIMER_ABSTIME CLOCK_REALTIME
 * timers to expire one second early during leap second.
 * See http://lwn.net/Articles/504658/.
 *
 * This is a regression test for the bug.
 *
 * Lingzhu Xiang <lxiang@redhat.com> Copyright (c) Red Hat, Inc., 2012.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "test.h"
#include "common_timers.h"

#define SECONDS_BEFORE_LEAP 2
#define SECONDS_AFTER_LEAP 2

char *TCID = "leapsec_timer";
int TST_TOTAL = 1;

static inline int in_order(struct timespec a, struct timespec b);
static void adjtimex_status(struct timex *tx, int status);
static const char *strtime(const struct timespec *now);
static void test_hrtimer_early_expiration(void);
static void run_leapsec(void);
static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		run_leapsec();
	}

	cleanup();
	tst_exit();
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
	int r;
	struct timespec now;

	tx->modes = ADJ_STATUS;
	tx->status = status;
	r = adjtimex(tx);
	now.tv_sec = tx->time.tv_sec;
	now.tv_nsec = tx->time.tv_usec * 1000;

	if ((tx->status & status) != status)
		tst_brkm(TBROK, cleanup, "adjtimex status %d not set", status);
	else if (r < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "adjtimex");
	else if (r < 6)
		tst_resm(TINFO, "%s adjtimex: %s", strtime(&now), msgs[r]);
	else
		tst_resm(TINFO, "%s adjtimex: clock state %d",
			 strtime(&now), r);
}

static const char *strtime(const struct timespec *now)
{
	static char fmt[256], buf[256];

	if (snprintf(fmt, sizeof(fmt), "%%F %%T.%09ld %%z", now->tv_nsec) < 0) {
		buf[0] = '\0';
		return buf;
	}
	if (!strftime(buf, sizeof(buf), fmt, localtime(&now->tv_sec))) {
		buf[0] = '\0';
		return buf;
	}
	return buf;
}

static void test_hrtimer_early_expiration(void)
{
	struct timespec now, target;
	int r, fail;

	clock_gettime(CLOCK_REALTIME, &now);
	tst_resm(TINFO, "now is     %s", strtime(&now));

	target = now;
	target.tv_sec++;
	tst_resm(TINFO, "sleep till %s", strtime(&target));
	r = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &target, NULL);
	if (r < 0) {
		tst_resm(TINFO | TERRNO, "clock_nanosleep");
		return;
	}

	clock_gettime(CLOCK_REALTIME, &now);
	tst_resm(TINFO, "now is     %s", strtime(&now));

	fail = !in_order(target, now);
	tst_resm(fail ? TFAIL : TINFO, "hrtimer early expiration is %s.",
		 fail ? "detected" : "not detected");
}

static void run_leapsec(void)
{
	const struct timespec sleeptime = { 0, NSEC_PER_SEC / 2 };
	struct timespec now, leap, start;
	struct timex tx;

	clock_gettime(CLOCK_REALTIME, &now);
	start = now;
	tst_resm(TINFO, "test start at %s", strtime(&now));

	test_hrtimer_early_expiration();

	/* calculate the next leap second */
	now.tv_sec += 86400 - now.tv_sec % 86400;
	now.tv_nsec = 0;
	leap = now;
	tst_resm(TINFO, "scheduling leap second %s", strtime(&leap));

	/* start before the leap second */
	now.tv_sec -= SECONDS_BEFORE_LEAP;
	if (clock_settime(CLOCK_REALTIME, &now) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clock_settime");
	tst_resm(TINFO, "setting time to        %s", strtime(&now));

	/* reset NTP time state */
	adjtimex_status(&tx, STA_PLL);
	adjtimex_status(&tx, 0);

	/* set the leap second insert flag */
	adjtimex_status(&tx, STA_INS);

	/* reliably sleep till after the leap second */
	while (tx.time.tv_sec < leap.tv_sec + SECONDS_AFTER_LEAP) {
		adjtimex_status(&tx, tx.status);
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL);
	}

	test_hrtimer_early_expiration();

	adjtimex_status(&tx, STA_PLL);
	adjtimex_status(&tx, 0);

	/* recover from timer expiring state and restore time */
	clock_gettime(CLOCK_REALTIME, &now);
	start.tv_sec += now.tv_sec - (leap.tv_sec - SECONDS_BEFORE_LEAP);
	start.tv_nsec += now.tv_nsec;
	start.tv_sec += start.tv_nsec / NSEC_PER_SEC;
	start.tv_nsec = start.tv_nsec % NSEC_PER_SEC;
	tst_resm(TINFO, "restoring time to %s", strtime(&start));
	/* calls clock_was_set() in kernel to revert inconsistency */
	if (clock_settime(CLOCK_REALTIME, &start) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clock_settime");

	test_hrtimer_early_expiration();
}

static void setup(void)
{
	tst_require_root();
	tst_sig(NOFORK, DEF_HANDLER, CLEANUP);
	TEST_PAUSE;
}

static void cleanup(void)
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	/* Calls clock_was_set() in kernel to revert inconsistency.
	 * The only possible EPERM doesn't matter here. */
	clock_settime(CLOCK_REALTIME, &now);
}
