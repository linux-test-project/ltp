// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright © International Business Machines  Corp., 2009
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * DESCRIPTION
 *      Glibc independent futex library for testing kernel functionality.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 */

#ifndef FUTEXTEST_H
#define FUTEXTEST_H

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include "lapi/futex.h"
#include "tst_timer.h"

#define FUTEX_INITIALIZER 0

enum futex_fn_type {
	FUTEX_FN_FUTEX,
	FUTEX_FN_FUTEX64,
};

struct futex_test_variants {
	enum futex_fn_type fntype;
	enum tst_ts_type tstype;
	int (*gettime)(clockid_t clk_id, void *ts);
	char *desc;
};

static inline void futex_supported_by_kernel(enum futex_fn_type fntype)
{
	if (fntype != FUTEX_FN_FUTEX64)
		return;

	/* Check if the syscall is implemented on the platform */
	TEST(sys_futex_time64(NULL, 0, 0, NULL, NULL, 0));
	if (TST_RET == -1 && TST_ERR == ENOSYS)
		tst_brk(TCONF, "Test not supported on kernel/platform");
}

/**
 * futex_syscall() - futex syscall wrapper
 * @fntype:	Futex function type
 * @uaddr:	address of first futex
 * @op:		futex op code
 * @val:	typically expected value of uaddr, but varies by op
 * @timeout:	typically an absolute struct tst_ts (except where noted
 *		otherwise). Overloaded by some ops
 * @uaddr2:	address of second futex for some ops\
 * @val3:	varies by op
 * @opflags:	flags to be bitwise OR'd with op, such as FUTEX_PRIVATE_FLAG
 *
 * futex_syscall() is used by all the following futex op wrappers. It can also be
 * used for misuse and abuse testing. Generally, the specific op wrappers
 * should be used instead. It is a macro instead of an static inline function as
 * some of the types over overloaded (timeout is used for nr_requeue for
 * example).
 *
 * These argument descriptions are the defaults for all
 * like-named arguments in the following wrappers except where noted below.
 */
static inline int futex_syscall(enum futex_fn_type fntype, futex_t *uaddr,
				int futex_op, futex_t val, void *timeout,
				futex_t *uaddr2, int val3, int opflags)
{
	int (*func)(int *uaddr, int futex_op, int val, void *to, int *uaddr2, int val3);

	if (fntype == FUTEX_FN_FUTEX)
		func = sys_futex;
	else
		func = sys_futex_time64;

	return func((int *)uaddr, futex_op | opflags, val, timeout, (int *)uaddr2, val3);
}

/**
 * futex_wait() - block on uaddr with optional timeout
 * @timeout:	relative timeout
 */
static inline int
futex_wait(enum futex_fn_type fntype, futex_t *uaddr, futex_t val,
	   struct tst_ts *timeout, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAIT, val,
			     tst_ts_get(timeout), NULL, 0, opflags);
}

/**
 * futex_wake() - wake one or more tasks blocked on uaddr
 * @nr_wake:	wake up to this many tasks
 */
static inline int
futex_wake(enum futex_fn_type fntype, futex_t *uaddr, int nr_wake, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAKE, nr_wake, NULL, NULL, 0,
			     opflags);
}

/**
 * futex_wait_bitset() - block on uaddr with bitset
 * @bitset:	bitset to be used with futex_wake_bitset
 */
static inline int
futex_wait_bitset(enum futex_fn_type fntype, futex_t *uaddr, futex_t val,
		  struct tst_ts *timeout, u_int32_t bitset, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAIT_BITSET, val,
			     tst_ts_get(timeout), NULL, bitset, opflags);
}

/**
 * futex_wake_bitset() - wake one or more tasks blocked on uaddr with bitset
 * @bitset:	bitset to compare with that used in futex_wait_bitset
 */
static inline int
futex_wake_bitset(enum futex_fn_type fntype, futex_t *uaddr, int nr_wake,
		  u_int32_t bitset, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAKE_BITSET, nr_wake, NULL,
			     NULL, bitset, opflags);
}

/**
 * futex_lock_pi() - block on uaddr as a PI mutex
 * @detect:	whether (1) or not (0) to perform deadlock detection
 */
static inline int
futex_lock_pi(enum futex_fn_type fntype, futex_t *uaddr, struct tst_ts *timeout,
	      int detect, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_LOCK_PI, detect,
			     tst_ts_get(timeout), NULL, 0, opflags);
}

/**
 * futex_unlock_pi() - release uaddr as a PI mutex, waking the top waiter
 */
static inline int
futex_unlock_pi(enum futex_fn_type fntype, futex_t *uaddr, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_UNLOCK_PI, 0, NULL, NULL, 0,
			     opflags); }

/**
 * futex_wake_op() - FIXME: COME UP WITH A GOOD ONE LINE DESCRIPTION
 */
static inline int
futex_wake_op(enum futex_fn_type fntype, futex_t *uaddr, futex_t *uaddr2,
	      int nr_wake, int nr_wake2, int wake_op, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAKE_OP, nr_wake,
			     (void *)((unsigned long)nr_wake2), uaddr2, wake_op,
			     opflags);
}

/**
 * futex_requeue() - requeue without expected value comparison, deprecated
 * @nr_wake:	wake up to this many tasks
 * @nr_requeue:	requeue up to this many tasks
 *
 * Due to its inherently racy implementation, futex_requeue() is deprecated in
 * favor of futex_cmp_requeue().
 */
static inline int
futex_requeue(enum futex_fn_type fntype, futex_t *uaddr, futex_t *uaddr2,
	      int nr_wake, int nr_requeue, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_REQUEUE, nr_wake,
			     (void *)((unsigned long)nr_requeue), uaddr2, 0,
			     opflags);
}

/**
 * futex_cmp_requeue() - requeue tasks from uaddr to uaddr2
 * @nr_wake:	wake up to this many tasks
 * @nr_requeue:	requeue up to this many tasks
 */
static inline int
futex_cmp_requeue(enum futex_fn_type fntype, futex_t *uaddr, futex_t val,
		  futex_t *uaddr2, int nr_wake, int nr_requeue, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_CMP_REQUEUE, nr_wake,
			     (void *)((unsigned long)nr_requeue), uaddr2, val,
			     opflags);
}

/**
 * futex_wait_requeue_pi() - block on uaddr and prepare to requeue to uaddr2
 * @uaddr:	non-PI futex source
 * @uaddr2:	PI futex target
 *
 * This is the first half of the requeue_pi mechanism. It shall always be
 * paired with futex_cmp_requeue_pi().
 */
static inline int
futex_wait_requeue_pi(enum futex_fn_type fntype, futex_t *uaddr, futex_t val,
		      futex_t *uaddr2, struct tst_ts *timeout, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_WAIT_REQUEUE_PI, val,
			     tst_ts_get(timeout), uaddr2, 0, opflags);
}

/**
 * futex_cmp_requeue_pi() - requeue tasks from uaddr to uaddr2 (PI aware)
 * @uaddr:	non-PI futex source
 * @uaddr2:	PI futex target
 * @nr_wake:	wake up to this many tasks
 * @nr_requeue:	requeue up to this many tasks
 */
static inline int
futex_cmp_requeue_pi(enum futex_fn_type fntype, futex_t *uaddr, futex_t val,
		     futex_t *uaddr2, int nr_wake, int nr_requeue, int opflags)
{
	return futex_syscall(fntype, uaddr, FUTEX_CMP_REQUEUE_PI, nr_wake,
			     (void *)((unsigned long)nr_requeue), uaddr2, val,
			     opflags);
}

/**
 * futex_cmpxchg() - atomic compare and exchange
 * @uaddr:	The address of the futex to be modified
 * @oldval:	The expected value of the futex
 * @newval:	The new value to try and assign the futex
 *
 * Implement cmpxchg using gcc atomic builtins.
 * http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html
 *
 * Return the old futex value.
 */
static inline u_int32_t
futex_cmpxchg(futex_t *uaddr, u_int32_t oldval, u_int32_t newval)
{
	return __sync_val_compare_and_swap(uaddr, oldval, newval);
}

/**
 * futex_dec() - atomic decrement of the futex value
 * @uaddr:	The address of the futex to be modified
 *
 * Return the new futex value.
 */
static inline u_int32_t
futex_dec(futex_t *uaddr)
{
	return __sync_sub_and_fetch(uaddr, 1);
}

/**
 * futex_inc() - atomic increment of the futex value
 * @uaddr:	the address of the futex to be modified
 *
 * Return the new futex value.
 */
static inline u_int32_t
futex_inc(futex_t *uaddr)
{
	return __sync_add_and_fetch(uaddr, 1);
}

/**
 * futex_set() - atomic decrement of the futex value
 * @uaddr:	the address of the futex to be modified
 * @newval:	New value for the atomic_t
 *
 * Return the new futex value.
 */
static inline u_int32_t
futex_set(futex_t *uaddr, u_int32_t newval)
{
	*uaddr = newval;
	return newval;
}

/**
 * futex_waked_someone() - ECHCK func for TST_RETRY_FUNC
 * @ret:	return value of futex_wake
 *
 * Return value drives TST_RETRY_FUNC macro.
 */
static inline int
futex_waked_someone(int ret)
{
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "futex_wake returned: %d", ret);

	return ret;
}

#endif /* _FUTEXTEST_H */
