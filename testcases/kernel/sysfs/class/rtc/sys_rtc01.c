// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the RTC attributes exported under
 * /sys/class/rtc/rtcN/.
 *
 * date and time report the RTC clock in UTC, while since_epoch reports the
 * same value pre-converted to a Unix timestamp. For every RTC device the
 * test verifies that:
 *
 * - hctosys is a boolean
 * - max_user_freq is greater than zero
 * - since_epoch matches date and time converted to a Unix timestamp
 * - since_epoch advances at the same rate as CLOCK_MONOTONIC_RAW
 *
 * The date/time vs since_epoch cross-check allows a few seconds of slack since
 * they are read from separate files and the wall clock keeps ticking in
 * between reads. The code also checks for midnight rollover and re-reads the
 * values if that happens.
 *
 * The RTC absolute value is not compared against the system time at all,
 * since there is no guarantee that the RTC is kept in sync with the system
 * clock. Instead the test polls since_epoch of all RTC devices for the test
 * runtime and records the CLOCK_MONOTONIC_RAW timestamps of the RTC second
 * transitions. Since each since_epoch read takes a non-zero amount of time,
 * every transition is known to have happened in an interval between the start
 * of the previous read and the end of the read that observed the new value.
 * For each pair of consecutive transitions the number of elapsed RTC seconds
 * is checked to be within the bounds of the CLOCK_MONOTONIC_RAW time elapsed
 * between the two intervals, plus a small tolerance for the RTC crystal
 * drift. The same check is then done for the whole interval between the first
 * and the last observed transition. The test also fails if since_epoch goes
 * backward, if it does not advance at least twice during the test runtime, or
 * if it stays at the same value for more than a second at the end of the
 * sampling.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

#define SLACK_SEC 10

/*
 * Generously high assumed worst-case RTC crystal drift, in parts per
 * million. 200 ppm covers even fairly poor/uncompensated RTC crystals across
 * a wide temperature range; a decent RTC typically drifts far less than this.
 */
#define MAX_RTC_DRIFT_PPM 200

/*
 * Fixed tolerance for the RTC vs CLOCK_MONOTONIC_RAW rate check, covers RTC
 * drivers that do not read the time atomically.
 */
#define RATE_TOLERANCE_US 10000

#define POLL_INTERVAL_US 1000

#define MAX_RTCS 10

struct rtc_edge {
	long long val;
	/* Bounds of the CLOCK_MONOTONIC_RAW time the transition happened in */
	struct timespec lo;
	struct timespec hi;
};

struct rtc_dev {
	char name[NAME_MAX + 1];

	long long last_val;
	int last_val_set;
	struct timespec last_start;

	int edges;
	int stalled;
	struct rtc_edge first;
	struct rtc_edge last;
};

static struct rtc_dev rtcs[MAX_RTCS];
static unsigned int rtc_cnt;

static void read_rtc_snapshot(const char *name, char *date, size_t date_size,
			      char *rtc_time, size_t time_size,
			      time_t *since_epoch)
{
	char date2[32];
	int i;

	/*
	 * Retry if the date changed between the reads -> midnight rollover
	 * happened.
	 */
	for (i = 0; i < 2; i++) {
		TST_SYSFS_READ_STR(date, date_size, PATH_CLASS_RTC "/%s/date", name);
		TST_SYSFS_READ_STR(rtc_time, time_size, PATH_CLASS_RTC "/%s/time",
				   name);

		*since_epoch = TST_SYSFS_READ_LLI(PATH_CLASS_RTC "/%s/since_epoch", name);

		TST_SYSFS_READ_STR(date2, sizeof(date2), PATH_CLASS_RTC "/%s/date",
				   name);

		if (!strcmp(date, date2))
			return;

		tst_res(TINFO,
			"%s: date rolled over mid-read (%s -> %s), retrying",
			name, date, date2);
	}

	tst_res(TWARN, "%s: date kept rolling over across the retry", name);
}

static void check_rtc_attrs(const char *name)
{
	char rtc_date[32] = "", rtc_time[32] = "";
	struct tm tm = { 0 };
	time_t rtc_epoch, since_epoch;

	TST_SYSFS_ASSERT_BOOL(PATH_CLASS_RTC "/%s/hctosys", name);
	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX, PATH_CLASS_RTC "/%s/max_user_freq", name);

	read_rtc_snapshot(name, rtc_date, sizeof(rtc_date), rtc_time, sizeof(rtc_time),
			  &since_epoch);

	if (sscanf(rtc_date, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) != 3 ||
	    sscanf(rtc_time, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 3) {
		tst_res(TFAIL, "%s: failed to parse date '%s' / time '%s'",
			name, rtc_date, rtc_time);
		return;
	}

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	rtc_epoch = timegm(&tm);

	if (llabs((long long)(rtc_epoch - since_epoch)) <= SLACK_SEC) {
		tst_res(TPASS,
			"%s: since_epoch (%lld) matches date '%s' time '%s' (%lld)",
			name, (long long)since_epoch, rtc_date, rtc_time,
			(long long)rtc_epoch);
	} else {
		tst_res(TFAIL,
			"%s: since_epoch (%lld) does not match date '%s' time '%s' (%lld)",
			name, (long long)since_epoch, rtc_date, rtc_time,
			(long long)rtc_epoch);
	}
}

static void check_edge_to_edge(const char *name, struct rtc_edge *from,
			       struct rtc_edge *to)
{
	long long rtc_us, min_us, max_us, tol_us;

	rtc_us = (to->val - from->val) * 1000000;
	min_us = tst_timespec_diff_us(to->lo, from->hi);
	max_us = tst_timespec_diff_us(to->hi, from->lo);
	tol_us = RATE_TOLERANCE_US + rtc_us / 1000000 * MAX_RTC_DRIFT_PPM;

	if (rtc_us >= min_us - tol_us && rtc_us <= max_us + tol_us) {
		tst_res(TPASS,
			"%s: %lli -> %lli took %9lldus - %9lldus = %lldus (tolerance %lldus)",
			name, from->val, to->val, min_us, max_us, max_us - min_us, tol_us);
	} else {
		tst_res(TFAIL,
			"%s: %lli -> %lli took %9lldus - %9lldus = %lldus (tolerance %lldus)",
			name, from->val, to->val, min_us, max_us, max_us - min_us, tol_us);
	}
}

static void poll_rtc(struct rtc_dev *rtc)
{
	struct timespec start, end;
	struct rtc_edge edge;
	long long val;

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &start);
	val = TST_SYSFS_READ_LLI(PATH_CLASS_RTC "/%s/since_epoch", rtc->name);
	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &end);

	if (rtc->last_val_set && val != rtc->last_val) {
		edge.val = val;
		edge.lo = rtc->last_start;
		edge.hi = end;

		if (val < rtc->last_val) {
			tst_res(TFAIL, "%s: since_epoch went backward %lli -> %lli",
				rtc->name, rtc->last_val, val);
		}

		if (rtc->edges > 0)
			check_edge_to_edge(rtc->name, &rtc->last, &edge);

		if (rtc->edges > 1)
			check_edge_to_edge(rtc->name, &rtc->first, &edge);

		if (!rtc->edges)
			rtc->first = edge;

		rtc->last = edge;
		rtc->edges++;
	}

	/*
	 * This read returned the value from the last transition, which happened
	 * at the latest at last.hi, so the RTC must not stay on it for longer
	 * than a second.
	 */
	if (rtc->edges && !rtc->stalled) {
		long long stall_us = tst_timespec_diff_us(start, rtc->last.hi);

		if (stall_us > 1000000 + RATE_TOLERANCE_US) {
			tst_res(TFAIL, "%s: since_epoch stuck at %lli for at least %lldus",
				rtc->name, rtc->last.val, stall_us);
			rtc->stalled = 1;
		}
	}

	rtc->last_val = val;
	rtc->last_val_set = 1;
	rtc->last_start = start;
}

static void do_test(void)
{
	unsigned int i;

	for (i = 0; i < rtc_cnt; i++) {
		tst_res(TINFO, "Checking %s attributes", rtcs[i].name);
		check_rtc_attrs(rtcs[i].name);

		rtcs[i].last_val_set = 0;
		rtcs[i].edges = 0;
		rtcs[i].stalled = 0;
	}

	tst_res(TINFO, "Sampling since_epoch against CLOCK_MONOTONIC_RAW");

	while (tst_remaining_runtime()) {
		for (i = 0; i < rtc_cnt; i++)
			poll_rtc(&rtcs[i]);

		usleep(POLL_INTERVAL_US);
	}

	for (i = 0; i < rtc_cnt; i++) {
		if (rtcs[i].edges < 2) {
			tst_res(TFAIL, "%s: since_epoch did not advance (%i transitions seen)",
				rtcs[i].name, rtcs[i].edges);
		}
	}
}

static void setup(void)
{
	struct dirent *ent;
	DIR *d;

	if (!tst_sysfs_exists(PATH_CLASS_RTC))
		tst_brk(TCONF, PATH_CLASS_RTC ": not present");

	d = SAFE_OPENDIR(PATH_CLASS_RTC);

	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "rtc", 3))
			continue;

		if (rtc_cnt >= MAX_RTCS)
			tst_brk(TBROK, "More than %i RTC devices found", MAX_RTCS);

		snprintf(rtcs[rtc_cnt].name, sizeof(rtcs[rtc_cnt].name), "%s",
			 ent->d_name);
		rtc_cnt++;
	}

	SAFE_CLOSEDIR(d);

	if (!rtc_cnt)
		tst_brk(TCONF, "No RTC device found");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_test,
	.min_runtime = 3,
	.runtime = 5,
};
