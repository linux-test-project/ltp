// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 */

#include <pthread.h>
#include <stdio.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int safe_pthread_create(const char *file, const int lineno,
			pthread_t *thread_id, const pthread_attr_t *attr,
			void *(*thread_fn)(void *), void *arg)
{
	int rval;

	rval = pthread_create(thread_id, attr, thread_fn, arg);

	if (rval) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_create(%p,%p,%p,%p) failed: %s", thread_id,
			 attr, thread_fn, arg, tst_strerrno(rval));
	}

	return rval;
}

int safe_pthread_join(const char *file, const int lineno,
		      pthread_t thread_id, void **retval)
{
	int rval;

	rval = pthread_join(thread_id, retval);

	if (rval) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_join(..., %p) failed: %s",
			 retval, tst_strerrno(rval));
	}

	return rval;
}

int safe_pthread_barrier_wait(const char *file, const int lineno,
			      pthread_barrier_t *barrier)
{
	int rval;

	rval =  pthread_barrier_wait(barrier);

	if (rval && rval != PTHREAD_BARRIER_SERIAL_THREAD) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_barrier_wait(%p) failed: %s",
			 barrier, tst_strerrno(rval));
	}

	return rval;
}

int safe_pthread_barrier_destroy(const char *file, const int lineno,
				 pthread_barrier_t *barrier)
{
	int rval;

	rval = pthread_barrier_destroy(barrier);

	if (rval) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_barrier_destroy(%p) failed: %s",
			 barrier, tst_strerrno(rval));
	}

	return rval;
}

int safe_pthread_cancel(const char *file, const int lineno,
			pthread_t thread_id)
{
	int rval;

	rval = pthread_cancel(thread_id);

	if (rval) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_cancel(...) failed: %s", tst_strerrno(rval));
	}

	return rval;
}

int safe_pthread_barrier_init(const char *file, const int lineno,
			      pthread_barrier_t *barrier,
			      const pthread_barrierattr_t *attr,
			      unsigned count)
{
	int rval;

	rval = pthread_barrier_init(barrier, attr, count);

	if (rval) {
		tst_brk_(file, lineno, TBROK,
			 "pthread_barrier_init(%p, %p, %u)failed: %s",
			 barrier, attr, count, tst_strerrno(rval));
	}

	return rval;
}
