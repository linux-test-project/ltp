/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 */

 /*

   Timer - struct timespec conversion runtimes and easy to use functions to
           measure elapsed time.

  */

#ifndef TST_TIMER
#define TST_TIMER

#include <sys/time.h>
#include <time.h>

static inline long long tst_timespec_to_ns(struct timespec t)
{
	return t.tv_sec * 1000000000 + t.tv_nsec;
}

/*
 * Converts timespec to microseconds.
 */
static inline long long tst_timespec_to_us(struct timespec t)
{
	return t.tv_sec * 1000000 + (t.tv_nsec + 500) / 1000;
}

/*
 * Converts timespec to milliseconds.
 */
static inline long long tst_timespec_to_ms(struct timespec t)
{
	return t.tv_sec * 1000 + (t.tv_nsec + 500000) / 1000000;
}

/*
 * Converts timeval to microseconds.
 */
static inline long long tst_timeval_to_us(struct timeval t)
{
	return t.tv_sec * 1000000 + t.tv_usec;
}

/*
 * Converts timeval to milliseconds.
 */
static inline long long tst_timeval_to_ms(struct timeval t)
{
	return t.tv_sec * 1000 + (t.tv_usec + 500) / 1000;
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
 * Converts ms to struct timespec
 */
static inline struct timespec tst_ms_to_timespec(long long ms)
{
	struct timespec ret;

	ret.tv_sec = ms / 1000;
	ret.tv_nsec = (ms % 1000) * 1000000;

	return ret;
}

/*
 * Converts us to struct timespec
 */
static inline struct timespec tst_us_to_timespec(long long us)
{
	struct timespec ret;

	ret.tv_sec = us / 1000000;
	ret.tv_nsec = (us % 1000000) * 1000;

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
 * Adds two timespec structures.
 */
static inline struct timespec tst_timespec_add(struct timespec t1,
                                               struct timespec t2)
{
	struct timespec res;

	res.tv_sec = t1.tv_sec + t2.tv_sec;
	res.tv_nsec = t1.tv_nsec + t2.tv_nsec;

	if (res.tv_nsec >= 1000000000) {
		res.tv_sec++;
		res.tv_nsec -= 1000000000;
	}

	return res;
}

/*
 * Subtracts us microseconds from t.
 */
static inline struct timespec tst_timespec_sub_us(struct timespec t,
                                                  long long us)
{
	t.tv_sec -= us / 1000000;
	t.tv_nsec -= (us % 1000000) * 1000;

	if (t.tv_nsec < 0) {
		t.tv_sec--;
		t.tv_nsec += 1000000000;
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

static inline long long tst_timespec_diff_ns(struct timespec t1,
					     struct timespec t2)
{
	return t1.tv_nsec - t2.tv_nsec + 1000000000LL * (t1.tv_sec - t2.tv_sec);
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
 * Returns difference between two timeval structures.
 */
static inline struct timeval tst_timeval_diff(struct timeval t1,
                                              struct timeval t2)
{
	struct timeval res;

	res.tv_sec = t1.tv_sec - t2.tv_sec;

	if (t1.tv_usec < t2.tv_usec) {
		res.tv_sec--;
		res.tv_usec = 1000000 - (t2.tv_usec - t1.tv_usec);
	} else {
		res.tv_usec = t1.tv_usec - t2.tv_usec;
	}

	return res;
}

static inline long long tst_timeval_diff_us(struct timeval t1,
                                            struct timeval t2)
{
	return tst_timeval_to_us(tst_timeval_diff(t1, t2));
}

static inline long long tst_timeval_diff_ms(struct timeval t1,
                                            struct timeval t2)
{
	return tst_timeval_to_ms(tst_timeval_diff(t1, t2));
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
 * Returns true if timer started by tst_timer_start() has been running for
 * longer than ms seconds.
 *
 * @ms: Time interval in milliseconds.
 */
int tst_timer_expired_ms(long long ms);

/*
 * Marks timer end time.
 */
void tst_timer_stop(void);

/*
 * Retuns elapsed time in struct timespec.
 */
struct timespec tst_timer_elapsed(void);

/*
 * Returns elapsed time in milliseconds.
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
