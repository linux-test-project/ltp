// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <time.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_timer.h"
#include "tst_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"
#include "lapi/common_timers.h"
#include "tst_kconfig.h"

typedef int (*mysyscall)(clockid_t clk_id, void *ts);

int syscall_supported_by_kernel(long sysnr)
{
	int ret;
	struct __kernel_timespec foo;

	ret = syscall(sysnr, 0, &foo);
	if (ret == -1 && errno == ENOSYS)
		return 0;

	return 1;
}

int tst_clock_getres(clockid_t clk_id, struct timespec *res)
{
	static struct tst_ts tts = { 0, };
	static mysyscall func;
	int ret;

#if (__NR_clock_getres_time64 != __LTP__NR_INVALID_SYSCALL)
	if (!func && syscall_supported_by_kernel(__NR_clock_getres_time64)) {
		func = sys_clock_getres64;
		tts.type = TST_KERN_TIMESPEC;
	}
#endif

	if (!func && syscall_supported_by_kernel(__NR_clock_getres)) {
		func = sys_clock_getres;
		tts.type = TST_KERN_OLD_TIMESPEC;
	}

	if (!func) {
		tst_res(TCONF, "clock_getres() not available");
		errno = ENOSYS;
		return -1;
	}

	ret = func(clk_id, tst_ts_get(&tts));
	res->tv_sec = tst_ts_get_sec(tts);
	res->tv_nsec = tst_ts_get_nsec(tts);
	return ret;
}

int tst_clock_gettime(clockid_t clk_id, struct timespec *ts)
{
	static struct tst_ts tts = { 0, };
	static mysyscall func;
	int ret;

#if (__NR_clock_gettime64 != __LTP__NR_INVALID_SYSCALL)
	if (!func && syscall_supported_by_kernel(__NR_clock_gettime64)) {
		func = sys_clock_gettime64;
		tts.type = TST_KERN_TIMESPEC;
	}
#endif

	if (!func && syscall_supported_by_kernel(__NR_clock_gettime)) {
		func = sys_clock_gettime;
		tts.type = TST_KERN_OLD_TIMESPEC;
	}

	if (!func) {
		tst_res(TCONF, "clock_gettime() not available");
		errno = ENOSYS;
		return -1;
	}

	ret = func(clk_id, tst_ts_get(&tts));
	ts->tv_sec = tst_ts_get_sec(tts);
	ts->tv_nsec = tst_ts_get_nsec(tts);
	return ret;
}

int tst_clock_settime(clockid_t clk_id, struct timespec *ts)
{
	static struct tst_ts tts = { 0, };
	static mysyscall func;

#if (__NR_clock_settime64 != __LTP__NR_INVALID_SYSCALL)
	if (!func && syscall_supported_by_kernel(__NR_clock_settime64)) {
		func = sys_clock_settime64;
		tts.type = TST_KERN_TIMESPEC;
	}
#endif

	if (!func && syscall_supported_by_kernel(__NR_clock_settime)) {
		func = sys_clock_settime;
		tts.type = TST_KERN_OLD_TIMESPEC;
	}

	if (!func) {
		tst_res(TCONF, "clock_settime() not available");
		errno = ENOSYS;
		return -1;
	}

	tst_ts_set_sec(&tts, ts->tv_sec);
	tst_ts_set_nsec(&tts, ts->tv_nsec);
	return func(clk_id, tst_ts_get(&tts));
}

const char *tst_clock_name(clockid_t clk_id)
{
	switch (clk_id) {
	case CLOCK_REALTIME:
		return "CLOCK_REALTIME";
	case CLOCK_MONOTONIC:
		return "CLOCK_MONOTONIC";
	case CLOCK_PROCESS_CPUTIME_ID:
		return "CLOCK_PROCESS_CPUTIME_ID";
	case CLOCK_THREAD_CPUTIME_ID:
		return "CLOCK_THREAD_CPUTIME_ID";
	case CLOCK_MONOTONIC_RAW:
		return "CLOCK_MONOTONIC_RAW";
	case CLOCK_REALTIME_COARSE:
		return "CLOCK_REALTIME_COARSE";
	case CLOCK_MONOTONIC_COARSE:
		return "CLOCK_MONOTONIC_COARSE";
	case CLOCK_BOOTTIME:
		return "CLOCK_BOOTTIME";
	case CLOCK_REALTIME_ALARM:
		return "CLOCK_REALTIME_ALARM";
	case CLOCK_BOOTTIME_ALARM:
		return "CLOCK_BOOTTIME_ALARM";
	case CLOCK_TAI:
		return "CLOCK_TAI";
	default:
		return "INVALID/UNKNOWN CLOCK";
	}
}

time_t tst_clock_get_timestamp(clockid_t clk_id)
{
	struct timespec ts;
	int ret;

	ret = tst_clock_gettime(clk_id, &ts);

	if (ret < 0) {
		tst_brk(TBROK | TERRNO, "clock_gettime(%s)",
			tst_clock_name(clk_id));
	}

	return ts.tv_sec;
}

time_t tst_fs_timestamp_start(void)
{
	return tst_clock_get_timestamp(CLOCK_REALTIME_COARSE);
}

time_t tst_fs_timestamp_end(void)
{
	return tst_clock_get_timestamp(CLOCK_REALTIME);
}

int tst_get_max_clocks(void)
{
	static const char * const kconf_aux[] = {"CONFIG_POSIX_AUX_CLOCKS=y", NULL};

	if (!tst_kconfig_check(kconf_aux))
		return MAX_CLOCKS + MAX_AUX_CLOCKS;
	else
		return MAX_CLOCKS;
}
