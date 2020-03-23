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

#endif /* TST_SAFE_PTHREAD_H__ */
