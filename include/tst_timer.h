/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

 /*

   Timer - struct timespec conversion runtimes and easy to use functions to
           measure elapsed time.

  */

#ifndef TST_TIMER
#define TST_TIMER

#include <sys/time.h>
#include <time.h>

/*
 * Converts timespec to microseconds.
 */
static inline long long tst_timespec_to_us(struct timespec t)
{
	return t.tv_sec * 1000000 + (t.tv_nsec + 500) / 1000;
}

/*
 * Converts timespec to miliseconds.
 */
static inline long long tst_timespec_to_ms(struct timespec t)
{
	return t.tv_sec * 1000 + (t.tv_nsec + 500000) / 1000000;
}

/*
 * Converts ms to struct timeval
 */
static inline struct timeval tst_ms_to_timeval(long long ms)
{
	struct timeval ret;

	ret.tv_sec = ms / 1000;
	ret.tv_usec = (ms % 1000) * 1000;

	return ret;
}

/*
 * Converts us to struct timeval
 */
static inline struct timeval tst_us_to_timeval(long long us)
{
	struct timeval ret;

	ret.tv_sec = us / 1000000;
	ret.tv_usec = us % 1000000;

	return ret;
}

/*
 * Comparsions
 */
static inline int tst_timespec_lt(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec == t2.tv_sec)
		return t1.tv_nsec < t2.tv_nsec;

	return t1.tv_sec < t2.tv_sec;
}

/*
 * Adds us microseconds to t.
 */
static inline struct timespec tst_timespec_add_us(struct timespec t,
                                                  long long us)
{
	t.tv_sec += us / 1000000;
	t.tv_nsec += (us % 1000000) * 1000;

	if (t.tv_nsec >= 1000000000) {
		t.tv_sec++;
		t.tv_nsec -= 1000000000;
	}

	return t;
}

/*
 * Returns difference between two timespec structures.
 */
static inline struct timespec tst_timespec_diff(struct timespec t1,
                                                struct timespec t2)
{
	struct timespec res;

	res.tv_sec = t1.tv_sec - t2.tv_sec;

	if (t1.tv_nsec < t2.tv_nsec) {
		res.tv_sec--;
		res.tv_nsec = 1000000000 - (t2.tv_nsec - t1.tv_nsec);
	} else {
		res.tv_nsec = t1.tv_nsec - t2.tv_nsec;
	}

	return res;
}

static inline long long tst_timespec_diff_us(struct timespec t1,
                                             struct timespec t2)
{
	return tst_timespec_to_us(tst_timespec_diff(t1, t2));
}

static inline long long tst_timespec_diff_ms(struct timespec t1,
                                             struct timespec t2)
{
	return tst_timespec_to_ms(tst_timespec_diff(t1, t2));
}

/*
 * Returns absolute value of difference between two timespec structures.
 */
static inline struct timespec tst_timespec_abs_diff(struct timespec t1,
                                                    struct timespec t2)
{
	if (tst_timespec_lt(t1, t2))
		return tst_timespec_diff(t2, t1);
	else
		return tst_timespec_diff(t1, t2);
}

static inline long long tst_timespec_abs_diff_us(struct timespec t1,
                                                 struct timespec t2)
{
       return tst_timespec_to_us(tst_timespec_abs_diff(t1, t2));
}

static inline long long tst_timespec_abs_diff_ms(struct timespec t1,
                                                 struct timespec t2)
{
       return tst_timespec_to_ms(tst_timespec_abs_diff(t1, t2));
}

/*
 * Exits the test with TCONF if particular timer is not supported. This is
 * intended to be used in test setup. There is no cleanup callback parameter as
 * you are expected to call it before initializing any resources that has to be
 * cleaned up later.
 *
 * @clk_id: Posix clock to use.
 */
void tst_timer_check(clockid_t clk_id);

/*
 * Marks a start time for given clock type.
 *
 * @clk_id: Posix clock to use.
 */
void tst_timer_start(clockid_t clk_id);

/*
 * Marks timer end time.
 */
void tst_timer_stop(void);

/*
 * Retuns elapsed time in struct timespec.
 */
struct timespec tst_timer_elapsed(void);

/*
 * Returns elapsed time in miliseconds.
 */
static inline long long tst_timer_elapsed_ms(void)
{
	return tst_timespec_to_ms(tst_timer_elapsed());
}

/*
 * Returns elapsed time in microseconds.
 */
static inline long long tst_timer_elapsed_us(void)
{
	return tst_timespec_to_us(tst_timer_elapsed());
}

#endif /* TST_TIMER */
