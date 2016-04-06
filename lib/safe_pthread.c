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
