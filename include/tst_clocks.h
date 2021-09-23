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
 * Returns current system time for file/IPC operations, which may slightly lag
 * behind time() return values.
 *
 * The reason for this is that the time() syscall reads the nanosecond timer at
 * the time of the call and adds it to the kernel current time, because of that
 * accumulation may cause it jump one second ahead compared to the kernel time
 * stamp that is used for IPC and filesystems.
 */
time_t tst_get_fs_timestamp(void);

#endif /* TST_CLOCKS__ */
