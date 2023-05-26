/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 */

#ifndef TST_SAFE_PTHREAD_H__
#define TST_SAFE_PTHREAD_H__

/*
 * Macro to use for making functions called only once in
 * multi-threaded tests such as init or cleanup function.
 * The first call to @name_fn function by any thread shall
 * call the @exec_fn. Subsequent calls shall not call @exec_fn.
 * *_fn functions must not take any arguments.
 */
#define TST_DECLARE_ONCE_FN(name_fn, exec_fn)				\
	void name_fn(void)						\
	{								\
		static pthread_once_t ltp_once = PTHREAD_ONCE_INIT;	\
		pthread_once(&ltp_once, exec_fn);			\
	}

int safe_pthread_create(const char *file, const int lineno,
			pthread_t *thread_id, const pthread_attr_t *attr,
			void *(*thread_fn)(void *), void *arg);
#define SAFE_PTHREAD_CREATE(thread_id, attr, thread_fn, arg) \
	safe_pthread_create(__FILE__, __LINE__, thread_id, attr, thread_fn, arg)

int safe_pthread_join(const char *file, const int lineno,
		      pthread_t thread_id, void **retval);
#define SAFE_PTHREAD_JOIN(thread_id, retval) \
	safe_pthread_join(__FILE__, __LINE__, thread_id, retval)

int safe_pthread_barrier_wait(const char *file, const int lineno,
			      pthread_barrier_t *barrier);
#define SAFE_PTHREAD_BARRIER_WAIT(barrier) \
	safe_pthread_barrier_wait(__FILE__, __LINE__, barrier);

int safe_pthread_barrier_destroy(const char *file, const int lineno,
				 pthread_barrier_t *barrier);
#define SAFE_PTHREAD_BARRIER_DESTROY(barrier) \
	safe_pthread_barrier_destroy(__FILE__, __LINE__, barrier);

int safe_pthread_barrier_init(const char *file, const int lineno,
			      pthread_barrier_t *barrier,
			      const pthread_barrierattr_t *attr,
			      unsigned count);
#define SAFE_PTHREAD_BARRIER_INIT(barrier, attr, count) \
	safe_pthread_barrier_init(__FILE__, __LINE__, barrier, attr, count);

int safe_pthread_cancel(const char *file, const int lineno,
			pthread_t thread_id);
#define SAFE_PTHREAD_CANCEL(thread_id) \
	safe_pthread_cancel(__FILE__, __LINE__, thread_id);

int safe_pthread_mutexattr_init(const char *file, const int lineno,
	pthread_mutexattr_t *attr);
#define SAFE_PTHREAD_MUTEXATTR_INIT(attr) \
	safe_pthread_mutexattr_init(__FILE__, __LINE__, (attr))

int safe_pthread_mutexattr_destroy(const char *file, const int lineno,
	pthread_mutexattr_t *attr);
#define SAFE_PTHREAD_MUTEXATTR_DESTROY(attr) \
	safe_pthread_mutexattr_destroy(__FILE__, __LINE__, (attr))

int safe_pthread_mutexattr_settype(const char *file, const int lineno,
	pthread_mutexattr_t *attr, int type);
#define SAFE_PTHREAD_MUTEXATTR_SETTYPE(attr, type) \
	safe_pthread_mutexattr_settype(__FILE__, __LINE__, (attr), (type))

int safe_pthread_mutex_init(const char *file, const int lineno,
	pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
#define SAFE_PTHREAD_MUTEX_INIT(mutex, attr) \
	safe_pthread_mutex_init(__FILE__, __LINE__, (mutex), (attr))

int safe_pthread_mutex_destroy(const char *file, const int lineno,
	pthread_mutex_t *mutex);
#define SAFE_PTHREAD_MUTEX_DESTROY(mutex) \
	safe_pthread_mutex_destroy(__FILE__, __LINE__, (mutex))

int safe_pthread_mutex_lock(const char *file, const int lineno,
	pthread_mutex_t *mutex);
#define SAFE_PTHREAD_MUTEX_LOCK(mutex) \
	safe_pthread_mutex_lock(__FILE__, __LINE__, (mutex))

/* Terminates the test on any error other than EBUSY */
int safe_pthread_mutex_trylock(const char *file, const int lineno,
	pthread_mutex_t *mutex);
#define SAFE_PTHREAD_MUTEX_TRYLOCK(mutex) \
	safe_pthread_mutex_trylock(__FILE__, __LINE__, (mutex))

/* Terminates the test on any error other than ETIMEDOUT */
int safe_pthread_mutex_timedlock(const char *file, const int lineno,
	pthread_mutex_t *mutex, const struct timespec *abstime);
#define SAFE_PTHREAD_MUTEX_TIMEDLOCK(mutex, abstime) \
	safe_pthread_mutex_timedlock(__FILE__, __LINE__, (mutex), (abstime))

int safe_pthread_mutex_unlock(const char *file, const int lineno,
	pthread_mutex_t *mutex);
#define SAFE_PTHREAD_MUTEX_UNLOCK(mutex) \
	safe_pthread_mutex_unlock(__FILE__, __LINE__, (mutex))

int safe_pthread_kill(const char *file, const int lineno,
	pthread_t thread, int sig);
#define SAFE_PTHREAD_KILL(thread, sig) \
	safe_pthread_kill(__FILE__, __LINE__, (thread), (sig))

#endif /* TST_SAFE_PTHREAD_H__ */
