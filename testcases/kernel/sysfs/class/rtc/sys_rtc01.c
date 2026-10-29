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
 * - for the RTC device with hctosys == 1, i.e. the one that was actually
 *   used by the kernel's own CONFIG_RTC_HCTOSYS code to set the system clock
 *   at boot, since_epoch is additionally close to the current system time(2)
 *   value. On a system with multiple RTC devices the other ones are not
 *   guaranteed to be in sync with the system clock at all (e.g. a
 *   peripheral's battery-backed RTC used only for its own wakeup alarm), so
 *   this second check is skipped for them.
 *
 * The hctosys only reflects whether the kernel itself performed the sync;
 * many setups runs ``hwclock --hctosys`` from a userspace init script instead
 * to sync the system clock from the RTC without the kernel ever knowing
 * about it, leaving hctosys == 0 on every device despite the clock
 * legitimately being in sync. To still get coverage on such systems, if no
 * RTC has hctosys == 1 at all, rtc0 is treated as the presumed primary RTC
 * and gets the system time check anyway.
 *
 * The date/time vs since_epoch cross-check allows a few seconds of slack since
 * they are read from separate files and the wall clock keeps ticking in
 * between reads. The code also checks for midnight rollover and re-reads the
 * values if that happens.
 *
 * The since_epoch vs system time(2) cross-check allows extra slack that
 * grows with system uptime: the RTC is a free-running hardware crystal
 * oscillator that (absent some periodic resync mechanism, which is not
 * guaranteed to be running) drifts independently of the system clock the
 * whole time the system is up, not just across the couple of reads this
 * test performs, so on a long-running system a purely fixed slack would
 * eventually produce false failures even on perfectly healthy hardware.
 *
 * The check may still fail if the system was booted with RTC that is fairly
 * off the current time and the system was later synced with NTP. This is out
 * of scope of this test.
 *
 * Under virtualization the slack is additionally multiplied, since a VM's
 * clock (and thus its emulated RTC) commonly lags real time by tens of
 * seconds right after boot due to hypervisor scheduling delays (steal time)
 * during the guest's own boot sequence, which is unrelated to physical RTC
 * crystal drift and not scaled away by the uptime-based slack above.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <sys/sysinfo.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

#define SLACK_SEC 10

/*
 * Generously high assumed worst-case RTC crystal drift, in parts per
 * million, used to scale the system time slack with uptime. 200 ppm covers
 * even fairly poor/uncompensated RTC crystals across a wide temperature
 * range; a decent RTC typically drifts far less than this.
 */
#define MAX_RTC_DRIFT_PPM 200

/*
 * VM guest clocks commonly lag behind real time by tens of seconds right
 * after boot due to hypervisor steal time accumulated during the guest's
 * own boot sequence, see the top comment. This is unrelated to physical RTC
 * crystal drift, hence a flat multiplier here rather than folding it into
 * MAX_RTC_DRIFT_PPM's uptime-based scaling.
 */
#define VIRT_SLACK_MULTIPLIER 10

static long sys_time_slack_sec(void)
{
	struct sysinfo info;
	long slack;

	SAFE_SYSINFO(&info);

	slack = SLACK_SEC + (long)info.uptime * MAX_RTC_DRIFT_PPM / 1000000;

	if (tst_is_virt(VIRT_ANY)) {
		tst_res(TINFO,
			"Running in a VM, multiplying the RTC vs system time slack by %d",
			VIRT_SLACK_MULTIPLIER);
		slack *= VIRT_SLACK_MULTIPLIER;
	}

	return slack;
}

static long read_hctosys(const char *name)
{
	return TST_SYSFS_READ_LI(PATH_CLASS_RTC "/%s/hctosys", name);
}

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

		*since_epoch = TST_SYSFS_READ_LI(PATH_CLASS_RTC "/%s/since_epoch", name);

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

static void check_rtc(const char *name, int any_hctosys)
{
	char date[32] = "", rtc_time[32] = "";
	struct tm tm = { 0 };
	time_t rtc_epoch, sys_epoch;
	long hctosys, slack;
	time_t since_epoch;
	int is_primary;

	TST_SYSFS_ASSERT_BOOL(PATH_CLASS_RTC "/%s/hctosys", name);
	TST_SYSFS_ASSERT_RANGELL(1, LONG_MAX, PATH_CLASS_RTC "/%s/max_user_freq", name);

	hctosys = read_hctosys(name);

	/*
	 * If no RTC has hctosys == 1 anywhere (see the top comment), fall
	 * back to treating rtc0 as the presumed primary RTC.
	 */
	is_primary = hctosys || (!any_hctosys && !strcmp(name, "rtc0"));

	read_rtc_snapshot(name, date, sizeof(date), rtc_time, sizeof(rtc_time),
			  &since_epoch);

	sys_epoch = time(NULL);

	if (sscanf(date, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday) != 3 ||
	    sscanf(rtc_time, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 3) {
		tst_res(TFAIL, "%s: failed to parse date '%s' / time '%s'",
			name, date, rtc_time);
		return;
	}

	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	rtc_epoch = timegm(&tm);

	if (llabs((long long)(rtc_epoch - since_epoch)) <= SLACK_SEC) {
		tst_res(TPASS,
			"%s: since_epoch (%lld) matches date '%s' time '%s' (%lld)",
			name, (long long)since_epoch, date, rtc_time,
			(long long)rtc_epoch);
	} else {
		tst_res(TFAIL,
			"%s: since_epoch (%lld) does not match date '%s' time '%s' (%lld)",
			name, (long long)since_epoch, date, rtc_time,
			(long long)rtc_epoch);
	}

	if (!is_primary) {
		tst_res(TINFO,
			"%s: not the (assumed) primary RTC, not checking since_epoch against system time",
			name);
		return;
	}

	slack = sys_time_slack_sec();
	long long diff = llabs((long long)(since_epoch - sys_epoch));

	if (diff <= slack) {
		tst_res(TPASS,
			"%s: since_epoch (%lld), system time (%lld), diff %llds <= slack %lds",
			name, (long long)since_epoch, (long long)sys_epoch,
			diff, slack);
	} else {
		tst_res(TFAIL,
			"%s: since_epoch (%lld), system time (%lld), diff %llds > slack %lds",
			name, (long long)since_epoch, (long long)sys_epoch,
			diff, slack);
	}
}

static void do_test(void)
{
	DIR *d;
	struct dirent *ent;
	int found = 0;
	int any_hctosys = 0;

	if (!tst_sysfs_exists(PATH_CLASS_RTC))
		tst_brk(TCONF, PATH_CLASS_RTC ": not present");

	d = SAFE_OPENDIR(PATH_CLASS_RTC);

	/* First pass: find out whether any RTC has hctosys == 1 already. */
	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "rtc", 3))
			continue;

		found = 1;

		if (read_hctosys(ent->d_name))
			any_hctosys = 1;
	}

	if (!found) {
		tst_res(TCONF, "No RTC device found");
		SAFE_CLOSEDIR(d);
		return;
	}

	rewinddir(d);

	/* Second pass: run the actual checks. */
	while ((ent = SAFE_READDIR(d))) {
		if (strncmp(ent->d_name, "rtc", 3))
			continue;

		tst_res(TINFO, "Checking %s", ent->d_name);
		check_rtc(ent->d_name, any_hctosys);
	}

	SAFE_CLOSEDIR(d);
}

static struct tst_test test = {
	.test_all = do_test,
};
