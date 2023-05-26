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

int safe_pthread_mutexattr_init(const char *file, const int lineno,
	pthread_mutexattr_t *attr)
{
	int ret;

	ret = pthread_mutexattr_init(attr);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutexattr_init(%p) failed: %s",
			attr, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutexattr_destroy(const char *file, const int lineno,
	pthread_mutexattr_t *attr)
{
	int ret;

	ret = pthread_mutexattr_destroy(attr);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutexattr_destroy(%p) failed: %s",
			attr, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutexattr_settype(const char *file, const int lineno,
	pthread_mutexattr_t *attr, int type)
{
	int ret;

	ret = pthread_mutexattr_settype(attr, type);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutexattr_settype(%p, %d) failed: %s",
			attr, type, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_init(const char *file, const int lineno,
	pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	int ret;

	ret = pthread_mutex_init(mutex, attr);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_init(%p, %p) failed: %s",
			mutex, attr, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_destroy(const char *file, const int lineno,
	pthread_mutex_t *mutex)
{
	int ret;

	ret = pthread_mutex_destroy(mutex);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_destroy(%p) failed: %s",
			mutex, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_lock(const char *file, const int lineno,
	pthread_mutex_t *mutex)
{
	int ret;

	ret = pthread_mutex_lock(mutex);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_lock(%p) failed: %s",
			mutex, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_trylock(const char *file, const int lineno,
	pthread_mutex_t *mutex)
{
	int ret;

	ret = pthread_mutex_trylock(mutex);

	if (ret && ret != EBUSY) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_trylock(%p) failed: %s",
			mutex, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_timedlock(const char *file, const int lineno,
	pthread_mutex_t *mutex, const struct timespec *abstime)
{
	int ret;

	ret = pthread_mutex_timedlock(mutex, abstime);

	if (ret && ret != ETIMEDOUT) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_timedlock(%p, {%lld, %ld}) failed: %s",
			mutex, (long long)abstime->tv_sec, abstime->tv_nsec,
			tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_mutex_unlock(const char *file, const int lineno,
	pthread_mutex_t *mutex)
{
	int ret;

	ret = pthread_mutex_unlock(mutex);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_mutex_unlock(%p) failed: %s",
			mutex, tst_strerrno(ret));
	}

	return ret;
}

int safe_pthread_kill(const char *file, const int lineno,
	pthread_t thread, int sig)
{
	int ret;

	ret = pthread_kill(thread, sig);

	if (ret) {
		tst_brk_(file, lineno, TBROK,
			"pthread_kill(..., %d) failed: %s",
			sig, tst_strerrno(ret));
	}

	return ret;
}
