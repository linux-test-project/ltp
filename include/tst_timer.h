/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2015-2020 Cyril Hrubis <chrubis@suse.cz>
 */

 /*

   Timer - struct timespec conversion runtimes and easy to use functions to
           measure elapsed time.

  */

#ifndef TST_TIMER
#define TST_TIMER

#include <sched.h>
#include <sys/time.h>
#include <mqueue.h>
#include <time.h>
#include "tst_test.h"
#include "lapi/common_timers.h"
#include "lapi/posix_types.h"
#include "lapi/syscalls.h"

/*
 * Converts timeval to microseconds.
 */
static inline long long tst_timeval_to_us(struct timeval t)
{
	return ((long long)t.tv_sec) * 1000000 + t.tv_usec;
}

/*
 * Converts timeval to milliseconds.
 */
static inline long long tst_timeval_to_ms(struct timeval t)
{
	return ((long long)t.tv_sec) * 1000 + (t.tv_usec + 500) / 1000;
}

/*
 * Converts milliseconds to struct timeval
 */
static inline struct timeval tst_ms_to_timeval(long long ms)
{
	struct timeval ret;

	ret.tv_sec = ms / 1000;
	ret.tv_usec = (ms % 1000) * 1000;

	return ret;
}

/*
 * Converts microseconds to struct timeval
 */
static inline struct timeval tst_us_to_timeval(long long us)
{
	struct timeval ret;

	ret.tv_sec = us / 1000000;
	ret.tv_usec = us % 1000000;

	return ret;
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

#ifndef __kernel_timespec

typedef __kernel_long_t	__kernel_old_time_t;

#ifndef HAVE_STRUCT___KERNEL_OLD_TIMEVAL
struct __kernel_old_timeval {
	__kernel_old_time_t	tv_sec;		/* seconds */
	__kernel_suseconds_t	tv_usec;	/* microseconds */
};
#endif

#ifndef HAVE_STRUCT___KERNEL_OLD_TIMESPEC
struct __kernel_old_timespec {
	__kernel_old_time_t	tv_sec;		/* seconds */
	__kernel_old_time_t	tv_nsec;	/* nanoseconds */
};
#endif

typedef long long __kernel_time64_t;

#ifndef HAVE_STRUCT___KERNEL_TIMESPEC
struct __kernel_timespec {
	__kernel_time64_t       tv_sec;                 /* seconds */
	long long               tv_nsec;                /* nanoseconds */
};
#endif

#ifndef HAVE_STRUCT___KERNEL_OLD_ITIMERSPEC
struct __kernel_old_itimerspec {
	struct __kernel_old_timespec it_interval;    /* timer period */
	struct __kernel_old_timespec it_value;       /* timer expiration */
};
#endif

#ifndef HAVE_STRUCT___KERNEL_ITIMERSPEC
struct __kernel_itimerspec {
	struct __kernel_timespec it_interval;    /* timer period */
	struct __kernel_timespec it_value;       /* timer expiration */
};
#endif

#ifndef HAVE_STRUCT___KERNEL_OLD_ITIMERVAL
struct __kernel_old_itimerval {
	struct __kernel_old_timeval it_interval;	/* timer interval */
	struct __kernel_old_timeval it_value;		/* current value */
};
#endif
#endif

enum tst_ts_type {
	TST_LIBC_TIMESPEC,
	TST_KERN_OLD_TIMESPEC,
	TST_KERN_TIMESPEC
};

struct tst_ts {
	enum tst_ts_type type;
	union ts {
		struct timespec libc_ts;
		struct __kernel_old_timespec kern_old_ts;
		struct __kernel_timespec kern_ts;
	} ts;
};

struct tst_its {
	enum tst_ts_type type;
	union {
		struct __kernel_old_itimerspec kern_old_its;
		struct __kernel_itimerspec kern_its;
	} ts;
};

static inline void *tst_ts_get(struct tst_ts *t)
{
	if (!t)
		return NULL;

	switch (t->type) {
	case TST_LIBC_TIMESPEC:
		return &t->ts.libc_ts;
	case TST_KERN_OLD_TIMESPEC:
		return &t->ts.kern_old_ts;
	case TST_KERN_TIMESPEC:
		return &t->ts.kern_ts;
	default:
		tst_brk(TBROK, "Invalid type: %d", t->type);
		return NULL;
	}
}

static inline void *tst_its_get(struct tst_its *t)
{
	if (!t)
		return NULL;

	switch (t->type) {
	case TST_KERN_OLD_TIMESPEC:
		return &t->ts.kern_old_its;
	case TST_KERN_TIMESPEC:
		return &t->ts.kern_its;
	default:
		tst_brk(TBROK, "Invalid type: %d", t->type);
		return NULL;
	}
}

static inline int libc_clock_getres(clockid_t clk_id, void *ts)
{
	return clock_getres(clk_id, ts);
}

static inline int sys_clock_getres(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_getres, clk_id, ts);
}

static inline int sys_clock_getres64(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_getres_time64, clk_id, ts);
}

static inline int libc_clock_gettime(clockid_t clk_id, void *ts)
{
	return clock_gettime(clk_id, ts);
}

static inline int sys_clock_gettime(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_gettime, clk_id, ts);
}

static inline int sys_clock_gettime64(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_gettime64, clk_id, ts);
}

static inline int libc_clock_settime(clockid_t clk_id, void *ts)
{
	return clock_settime(clk_id, ts);
}

static inline int sys_clock_settime(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_settime, clk_id, ts);
}

static inline int sys_clock_settime64(clockid_t clk_id, void *ts)
{
	return tst_syscall(__NR_clock_settime64, clk_id, ts);
}

static inline int libc_clock_nanosleep(clockid_t clk_id, int flags,
				       void *request, void *remain)
{
	return clock_nanosleep(clk_id, flags, request, remain);
}

static inline int sys_clock_nanosleep(clockid_t clk_id, int flags,
				      void *request, void *remain)
{
	return tst_syscall(__NR_clock_nanosleep, clk_id, flags,
			   request, remain);
}

static inline int sys_clock_nanosleep64(clockid_t clk_id, int flags,
				        void *request, void *remain)
{
	return tst_syscall(__NR_clock_nanosleep_time64, clk_id, flags,
			   request, remain);
}

static inline int sys_futex(int *uaddr, int futex_op, int val, void *to,
			    int *uaddr2, int val3)
{
	return tst_syscall(__NR_futex, uaddr, futex_op, val, to, uaddr2, val3);
}

static inline int sys_futex_time64(int *uaddr, int futex_op, int val, void *to,
				   int *uaddr2, int val3)
{
	return tst_syscall(__NR_futex_time64, uaddr, futex_op, val, to, uaddr2, val3);
}

static inline int libc_mq_timedsend(mqd_t mqdes, const char *msg_ptr,
		size_t msg_len, unsigned int msg_prio, void *abs_timeout)
{
	return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

static inline int sys_mq_timedsend(mqd_t mqdes, const char *msg_ptr,
		size_t msg_len, unsigned int msg_prio, void *abs_timeout)
{
	return tst_syscall(__NR_mq_timedsend, mqdes, msg_ptr, msg_len, msg_prio,
			   abs_timeout);
}

static inline int sys_mq_timedsend64(mqd_t mqdes, const char *msg_ptr,
		size_t msg_len, unsigned int msg_prio, void *abs_timeout)
{
	return tst_syscall(__NR_mq_timedsend_time64, mqdes, msg_ptr, msg_len,
			   msg_prio, abs_timeout);
}

static inline ssize_t libc_mq_timedreceive(mqd_t mqdes, char *msg_ptr,
		size_t msg_len, unsigned int *msg_prio, void *abs_timeout)
{
	return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

static inline ssize_t sys_mq_timedreceive(mqd_t mqdes, char *msg_ptr,
		size_t msg_len, unsigned int *msg_prio, void *abs_timeout)
{
	return tst_syscall(__NR_mq_timedreceive, mqdes, msg_ptr, msg_len,
			   msg_prio, abs_timeout);
}

static inline ssize_t sys_mq_timedreceive64(mqd_t mqdes, char *msg_ptr,
		size_t msg_len, unsigned int *msg_prio, void *abs_timeout)
{
	return tst_syscall(__NR_mq_timedreceive_time64, mqdes, msg_ptr, msg_len,
			   msg_prio, abs_timeout);
}

static inline int libc_sched_rr_get_interval(pid_t pid, void *ts)
{
	return sched_rr_get_interval(pid, ts);
}

static inline int sys_sched_rr_get_interval(pid_t pid, void *ts)
{
	return tst_syscall(__NR_sched_rr_get_interval, pid, ts);
}

static inline int sys_sched_rr_get_interval64(pid_t pid, void *ts)
{
	return tst_syscall(__NR_sched_rr_get_interval_time64, pid, ts);
}

static inline int sys_timer_gettime(kernel_timer_t timerid, void *its)
{
	return tst_syscall(__NR_timer_gettime, timerid, its);
}

static inline int sys_timer_gettime64(kernel_timer_t timerid, void *its)
{
	return tst_syscall(__NR_timer_gettime64, timerid, its);
}

static inline int sys_timer_settime(kernel_timer_t timerid, int flags, void *its,
				    void *old_its)
{
	return tst_syscall(__NR_timer_settime, timerid, flags, its, old_its);
}

static inline int sys_timer_settime64(kernel_timer_t timerid, int flags, void *its,
				      void *old_its)
{
	return tst_syscall(__NR_timer_settime64, timerid, flags, its, old_its);
}

static inline int sys_timerfd_gettime(int fd, void *its)
{
	return tst_syscall(__NR_timerfd_gettime, fd, its);
}

static inline int sys_timerfd_gettime64(int fd, void *its)
{
	return tst_syscall(__NR_timerfd_gettime64, fd, its);
}

static inline int sys_timerfd_settime(int fd, int flags, void *its,
				      void *old_its)
{
	return tst_syscall(__NR_timerfd_settime, fd, flags, its, old_its);
}

static inline int sys_timerfd_settime64(int fd, int flags, void *its,
				      void *old_its)
{
	return tst_syscall(__NR_timerfd_settime64, fd, flags, its, old_its);
}

static inline int sys_setitimer(int which, void *new_value, void *old_value)
{
	return tst_syscall(__NR_setitimer, which, new_value, old_value);
}

/*
 * Returns tst_ts seconds.
 */
static inline long long tst_ts_get_sec(struct tst_ts ts)
{
	switch (ts.type) {
	case TST_LIBC_TIMESPEC:
		return ts.ts.libc_ts.tv_sec;
	case TST_KERN_OLD_TIMESPEC:
		return ts.ts.kern_old_ts.tv_sec;
	case TST_KERN_TIMESPEC:
		return ts.ts.kern_ts.tv_sec;
	default:
		tst_brk(TBROK, "Invalid type: %d", ts.type);
		return -1;
	}
}

/*
 * Returns tst_ts nanoseconds.
 */
static inline long long tst_ts_get_nsec(struct tst_ts ts)
{
	switch (ts.type) {
	case TST_LIBC_TIMESPEC:
		return ts.ts.libc_ts.tv_nsec;
	case TST_KERN_OLD_TIMESPEC:
		return ts.ts.kern_old_ts.tv_nsec;
	case TST_KERN_TIMESPEC:
		return ts.ts.kern_ts.tv_nsec;
	default:
		tst_brk(TBROK, "Invalid type: %d", ts.type);
		return -1;
	}
}

/*
 * Sets tst_ts seconds.
 */
static inline void tst_ts_set_sec(struct tst_ts *ts, long long sec)
{
	switch (ts->type) {
	case TST_LIBC_TIMESPEC:
		ts->ts.libc_ts.tv_sec = sec;
	break;
	case TST_KERN_OLD_TIMESPEC:
		ts->ts.kern_old_ts.tv_sec = sec;
	break;
	case TST_KERN_TIMESPEC:
		ts->ts.kern_ts.tv_sec = sec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", ts->type);
	}
}

/*
 * Sets tst_ts nanoseconds.
 */
static inline void tst_ts_set_nsec(struct tst_ts *ts, long long nsec)
{
	switch (ts->type) {
	case TST_LIBC_TIMESPEC:
		ts->ts.libc_ts.tv_nsec = nsec;
	break;
	case TST_KERN_OLD_TIMESPEC:
		ts->ts.kern_old_ts.tv_nsec = nsec;
	break;
	case TST_KERN_TIMESPEC:
		ts->ts.kern_ts.tv_nsec = nsec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", ts->type);
	}
}

/*
 * Returns tst_its it_interval seconds.
 */
static inline long long tst_its_get_interval_sec(struct tst_its its)
{
	switch (its.type) {
	case TST_KERN_OLD_TIMESPEC:
		return its.ts.kern_old_its.it_interval.tv_sec;
	case TST_KERN_TIMESPEC:
		return its.ts.kern_its.it_interval.tv_sec;
	default:
		tst_brk(TBROK, "Invalid type: %d", its.type);
		return -1;
	}
}

/*
 * Returns tst_its it_interval nanoseconds.
 */
static inline long long tst_its_get_interval_nsec(struct tst_its its)
{
	switch (its.type) {
	case TST_KERN_OLD_TIMESPEC:
		return its.ts.kern_old_its.it_interval.tv_nsec;
	case TST_KERN_TIMESPEC:
		return its.ts.kern_its.it_interval.tv_nsec;
	default:
		tst_brk(TBROK, "Invalid type: %d", its.type);
		return -1;
	}
}

/*
 * Sets tst_its it_interval seconds.
 */
static inline void tst_its_set_interval_sec(struct tst_its *its, long long sec)
{
	switch (its->type) {
	break;
	case TST_KERN_OLD_TIMESPEC:
		its->ts.kern_old_its.it_interval.tv_sec = sec;
	break;
	case TST_KERN_TIMESPEC:
		its->ts.kern_its.it_interval.tv_sec = sec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", its->type);
	}
}

/*
 * Sets tst_its it_interval nanoseconds.
 */
static inline void tst_its_set_interval_nsec(struct tst_its *its, long long nsec)
{
	switch (its->type) {
	break;
	case TST_KERN_OLD_TIMESPEC:
		its->ts.kern_old_its.it_interval.tv_nsec = nsec;
	break;
	case TST_KERN_TIMESPEC:
		its->ts.kern_its.it_interval.tv_nsec = nsec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", its->type);
	}
}

/*
 * Returns tst_its it_value seconds.
 */
static inline long long tst_its_get_value_sec(struct tst_its its)
{
	switch (its.type) {
	case TST_KERN_OLD_TIMESPEC:
		return its.ts.kern_old_its.it_value.tv_sec;
	case TST_KERN_TIMESPEC:
		return its.ts.kern_its.it_value.tv_sec;
	default:
		tst_brk(TBROK, "Invalid type: %d", its.type);
		return -1;
	}
}

/*
 * Returns tst_its it_value nanoseconds.
 */
static inline long long tst_its_get_value_nsec(struct tst_its its)
{
	switch (its.type) {
	case TST_KERN_OLD_TIMESPEC:
		return its.ts.kern_old_its.it_value.tv_nsec;
	case TST_KERN_TIMESPEC:
		return its.ts.kern_its.it_value.tv_nsec;
	default:
		tst_brk(TBROK, "Invalid type: %d", its.type);
		return -1;
	}
}

/*
 * Sets tst_its it_value seconds.
 */
static inline void tst_its_set_value_sec(struct tst_its *its, long long sec)
{
	switch (its->type) {
	break;
	case TST_KERN_OLD_TIMESPEC:
		its->ts.kern_old_its.it_value.tv_sec = sec;
	break;
	case TST_KERN_TIMESPEC:
		its->ts.kern_its.it_value.tv_sec = sec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", its->type);
	}
}

/*
 * Sets tst_its it_value nanoseconds.
 */
static inline void tst_its_set_value_nsec(struct tst_its *its, long long nsec)
{
	switch (its->type) {
	break;
	case TST_KERN_OLD_TIMESPEC:
		its->ts.kern_old_its.it_value.tv_nsec = nsec;
	break;
	case TST_KERN_TIMESPEC:
		its->ts.kern_its.it_value.tv_nsec = nsec;
	break;
	default:
		tst_brk(TBROK, "Invalid type: %d", its->type);
	}
}

/*
 * Checks that timespec is valid, i.e. that the timestamp is not zero and that
 * the nanoseconds are normalized i.e. in <0, 1s) interval.
 *
 *  0: On success, i.e. timespec updated correctly.
 * -1: Error, timespec not updated.
 * -2: Error, tv_nsec is corrupted.
 */
static inline int tst_ts_valid(struct tst_ts *t)
{
	long long nsec = tst_ts_get_nsec(*t);

	if (nsec < 0 || nsec >= 1000000000)
		return -2;

	if (tst_ts_get_sec(*t) == 0 && tst_ts_get_nsec(*t) == 0)
		return -1;

	return 0;
}

/*
 * Converts timespec to tst_ts.
 */
static inline struct tst_ts tst_ts_from_timespec(struct timespec ts)
{
	struct tst_ts t = {
		.type = TST_LIBC_TIMESPEC,
		.ts.libc_ts.tv_sec = ts.tv_sec,
		.ts.libc_ts.tv_nsec = ts.tv_nsec,
	};

	return t;
}

/*
 * Converst tst_ts into timespec.
 */
static inline struct timespec tst_ts_to_timespec(struct tst_ts t)
{
	return t.ts.libc_ts;
}

/*
 * Converts tst_ts to nanoseconds.
 */
static inline long long tst_ts_to_ns(struct tst_ts t)
{
	return tst_ts_get_sec(t) * 1000000000 + tst_ts_get_nsec(t);
}

/*
 * Converts tst_ts to microseconds and rounds the value.
 */
static inline long long tst_ts_to_us(struct tst_ts t)
{
	return tst_ts_get_sec(t) * 1000000 +
	       (tst_ts_get_nsec(t) + 500) / 1000;
}

/*
 * Converts timespec to microseconds and rounds the value.
 */
static inline long long tst_timespec_to_us(struct timespec ts)
{
	return tst_ts_to_us(tst_ts_from_timespec(ts));
}

/*
 * Converts tst_ts to milliseconds and rounds the value.
 */
static inline long long tst_ts_to_ms(struct tst_ts t)
{
	return tst_ts_get_sec(t) * 1000 +
	       (tst_ts_get_nsec(t) + 500000) / 1000000;
}

/*
 * Converts timespec to milliseconds and rounds the value.
 */
static inline long long tst_timespec_to_ms(struct timespec ts)
{
	return tst_ts_to_ms(tst_ts_from_timespec(ts));
}

/*
 * Converts nanoseconds to tst_ts
 */
static inline struct tst_ts
tst_ts_from_ns(enum tst_ts_type type, long long ns)
{
	struct tst_ts ret = {.type = type};

	tst_ts_set_sec(&ret, ns / 1000000000);
	tst_ts_set_nsec(&ret, ns % 1000000000);

	return ret;
}

/*
 * Converts microseconds to tst_ts
 */
static inline struct tst_ts
tst_ts_from_us(enum tst_ts_type type, long long us)
{
	struct tst_ts ret = {.type = type};

	tst_ts_set_sec(&ret, us / 1000000);
	tst_ts_set_nsec(&ret, (us % 1000000) * 1000);

	return ret;
}

/*
 * Converts microseconds to timespec
 */
static inline struct timespec
tst_timespec_from_us(long long us)
{
	return tst_ts_to_timespec(tst_ts_from_us(TST_LIBC_TIMESPEC, us));
}

/*
 * Converts miliseconds to tst_ts
 */
static inline struct tst_ts
tst_ts_from_ms(enum tst_ts_type type, long long ms)
{
	struct tst_ts ret = {.type = type};

	tst_ts_set_sec(&ret, ms / 1000);
	tst_ts_set_nsec(&ret, (ms % 1000) * 1000000);

	return ret;
}

/*
 * Converts miliseconds to timespec
 */
static inline struct timespec
tst_timespec_from_ms(long long ms)
{
	return tst_ts_to_timespec(tst_ts_from_ms(TST_LIBC_TIMESPEC, ms));
}

/*
 * Sets tst_its it_value from microseconds.
 */
static inline void tst_its_set_interval_from_us(struct tst_its *its, long long usec)
{
	struct timespec tp = tst_timespec_from_us(usec);

	tst_its_set_interval_sec(its, tp.tv_sec);
	tst_its_set_interval_nsec(its, tp.tv_nsec);
}

/*
 * Sets tst_its it_value from microseconds.
 */
static inline void tst_its_set_value_from_us(struct tst_its *its, long long usec)
{
	struct timespec tp = tst_timespec_from_us(usec);

	tst_its_set_value_sec(its, tp.tv_sec);
	tst_its_set_value_nsec(its, tp.tv_nsec);
}

/*
 * Sets tst_its it_interval from tst_ts.
 */
static inline void tst_its_set_interval_from_ts(struct tst_its *its, struct tst_ts ts)
{
	tst_its_set_interval_sec(its, tst_ts_get_sec(ts));
	tst_its_set_interval_nsec(its, tst_ts_get_nsec(ts));
}

/*
 * Sets tst_its it_value from tst_ts.
 */
static inline void tst_its_set_value_from_ts(struct tst_its *its, struct tst_ts ts)
{
	tst_its_set_value_sec(its, tst_ts_get_sec(ts));
	tst_its_set_value_nsec(its, tst_ts_get_nsec(ts));
}

/*
 * Returns if t1 less than t2. Both t1 and t2 must be normalized.
 */
static inline int tst_ts_lt(struct tst_ts t1, struct tst_ts t2)
{
	if (tst_ts_get_sec(t1) == tst_ts_get_sec(t2))
		return tst_ts_get_nsec(t1) < tst_ts_get_nsec(t2);

	return tst_ts_get_sec(t1) < tst_ts_get_sec(t2);
}

/*
 * Returns if ts1 less than ts2. Both ts1 and ts2 must be normalized.
 */
static inline int tst_timespec_lt(struct timespec ts1, struct timespec ts2)
{
	return tst_ts_lt(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));
}

/*
 * Returns normalized tst_ts, i.e. 0 <= nsec < 1000000000.
 */
static inline struct tst_ts tst_ts_normalize(struct tst_ts t)
{
	long long sec = tst_ts_get_sec(t);
	long long nsec = tst_ts_get_nsec(t);

	if (nsec >= 1000000000) {
		tst_ts_set_sec(&t, sec + 1);
		tst_ts_set_nsec(&t, nsec - 1000000000);
	}

	if (nsec < 0) {
		tst_ts_set_sec(&t, sec - 1);
		tst_ts_set_nsec(&t, nsec + 1000000000);
	}

	return t;
}

/*
 * Adds us microseconds to tst_ts.
 */
static inline struct tst_ts
tst_ts_add_us(struct tst_ts t, long long us)
{
	struct tst_ts res = {.type = t.type};

	tst_ts_set_sec(&res, tst_ts_get_sec(t) + us / 1000000);
	tst_ts_set_nsec(&res, tst_ts_get_nsec(t) + (us % 1000000) * 1000);

	return tst_ts_normalize(res);
}

/*
 * Adds us microseconds to struct timespec.
 */
static inline struct timespec
tst_timespec_add_us(struct timespec ts, long long us)
{
	struct tst_ts res;

	res = tst_ts_add_us(tst_ts_from_timespec(ts), us);

	return tst_ts_to_timespec(res);
}

/*
 * Substracts us microseconds from tst_ts.
 */
static inline struct tst_ts
tst_ts_sub_us(struct tst_ts t, long long us)
{
	struct tst_ts res = {.type = t.type};

	tst_ts_set_sec(&res, tst_ts_get_sec(t) - us / 1000000);
	tst_ts_set_nsec(&res, tst_ts_get_nsec(t) - (us % 1000000) * 1000);

	return tst_ts_normalize(res);
}

/*
 * Substracts us microseconds from timespec.
 */
static inline struct timespec
tst_timespec_sub_us(struct timespec ts, long long us)
{
	struct tst_ts res;

	res = tst_ts_sub_us(tst_ts_from_timespec(ts), us);

	return tst_ts_to_timespec(res);
}

/*
 * Adds two tst_ts structures.
 */
static inline struct tst_ts
tst_ts_add(struct tst_ts t1, struct tst_ts t2)
{
	struct tst_ts res = {.type = t1.type};

	tst_ts_set_sec(&res, tst_ts_get_sec(t1) + tst_ts_get_sec(t2));
	tst_ts_set_nsec(&res, tst_ts_get_nsec(t1) + tst_ts_get_nsec(t2));

	return tst_ts_normalize(res);
}

/*
 * Adds two timespec structures.
 */
static inline struct timespec
tst_timespec_add(struct timespec ts1, struct timespec ts2)
{
	struct tst_ts res;

	res = tst_ts_add(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));

	return tst_ts_to_timespec(res);
}

/*
 * Substract two tst_ts structures.
 */
static inline struct tst_ts
tst_ts_diff(struct tst_ts t1, struct tst_ts t2)
{
	struct tst_ts res = {.type = t1.type};

	tst_ts_set_sec(&res, tst_ts_get_sec(t1) - tst_ts_get_sec(t2));
	tst_ts_set_nsec(&res, tst_ts_get_nsec(t1) - tst_ts_get_nsec(t2));

	return tst_ts_normalize(res);
}

/*
 * Substract two timespec structures.
 */
static inline struct timespec
tst_timespec_diff(struct timespec ts1, struct timespec ts2)
{
	struct tst_ts res;

	res = tst_ts_diff(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));

	return tst_ts_to_timespec(res);
}

/*
 * Substract two tst_ts structures returns number of nanoseconds.
 */
static inline long long
tst_ts_diff_ns(struct tst_ts t1, struct tst_ts t2)
{
	return tst_ts_to_ns(tst_ts_diff(t1, t2));
}

/*
 * Substract two timespec structures returns number of nanoseconds.
 */
static inline long long
tst_timespec_diff_ns(struct timespec ts1, struct timespec ts2)
{
	return tst_ts_diff_ns(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));
}

/*
 * Substract two tst_ts structures returns number of microseconds.
 */
static inline long long
tst_ts_diff_us(struct tst_ts t1, struct tst_ts t2)
{
	return tst_ts_to_us(tst_ts_diff(t1, t2));
}

/*
 * Substract two timespec structures returns number of microseconds.
 */
static inline long long
tst_timespec_diff_us(struct timespec ts1, struct timespec ts2)
{
	return tst_ts_diff_us(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));
}

/*
 * Substract two tst_ts structures returns number of milliseconds.
 */
static inline long long
tst_ts_diff_ms(struct tst_ts t1, struct tst_ts t2)
{
	return tst_ts_to_ms(tst_ts_diff(t1, t2));
}

/*
 * Substract two timespec structures returns number of milliseconds.
 */
static inline long long
tst_timespec_diff_ms(struct timespec ts1, struct timespec ts2)
{
	return tst_ts_diff_ms(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));
}

/*
 * Returns absolute value of difference between two timespec structures.
 */
static inline struct tst_ts
tst_ts_abs_diff(struct tst_ts t1, struct tst_ts t2)
{
	if (tst_ts_lt(t1, t2))
		return tst_ts_diff(t2, t1);
	else
		return tst_ts_diff(t1, t2);
}

/*
 * Returns absolute value of difference between two tst_ts structures in
 * microseconds.
 */
static inline long long
tst_ts_abs_diff_us(struct tst_ts t1, struct tst_ts t2)
{
	return tst_ts_to_us(tst_ts_abs_diff(t1, t2));
}

/*
 * Returns absolute value of difference between two timespec structures in
 * microseconds.
 */
static inline long long
tst_timespec_abs_diff_us(struct timespec ts1, struct timespec ts2)
{
	return tst_ts_abs_diff_us(tst_ts_from_timespec(ts1), tst_ts_from_timespec(ts2));
}

/*
 * Returns absolute value of difference between two timespec structures in
 * milliseconds.
 */
static inline long long
tst_ts_abs_diff_ms(struct tst_ts t1, struct tst_ts t2)
{
	return tst_ts_to_ms(tst_ts_abs_diff(t1, t2));
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
