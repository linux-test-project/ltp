/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * clock_gettime() and clock_getres() functions
 */

#ifndef TST_CLOCKS__
#define TST_CLOCKS__

int tst_clock_getres(clockid_t clk_id, struct timespec *res);

int tst_clock_gettime(clockid_t clk_id, struct timespec *ts);

int tst_clock_settime(clockid_t clk_id, struct timespec *ts);

/*
 * Converts clock id to a readable name.
 */
const char *tst_clock_name(clockid_t clk_id);

/*
 * Returns timestamp (seconds passed) for the specified clock.
 * TBROKs on error.
 */
time_t tst_clock_get_timestamp(clockid_t clk_id);

/*
 * Returns current system time for file/IPC operations, which may slightly lag
 * behind time() return values. Meant to be used as lower bound in atime/mtime
 * checks.
 *
 * The reason for this is that the time() syscall reads the nanosecond timer at
 * the time of the call and adds it to the kernel current time, because of that
 * accumulation may cause it jump one second ahead compared to the kernel time
 * stamp that is used for IPC and filesystems.
 */
time_t tst_fs_timestamp_start(void);

/*
 * Returns current system time for file/IPC operation, using clock
 * which has higher precision. Meant to be used as higher bound in atime/mtime
 * checks.
 *
 * The reason for separate start/end functions is to cover features like
 * multigrain timestamps, which update atime/mtime using more precise clock.
 */
time_t tst_fs_timestamp_end(void);

int tst_get_max_clocks(void);

#endif /* TST_CLOCKS__ */
