/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
