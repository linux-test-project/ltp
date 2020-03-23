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

#endif /* TST_CLOCKS__ */
