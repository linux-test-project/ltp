  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test this function is defined:

   pthread_create(pthread_t *restrict, const pthread_attr_t *restrict,
   void *(*)(void *), void *restrict);
   */

#include <pthread.h>
#include "posixtest.h"

void *thread_function(void *arg LTP_ATTRIBUTE_UNUSED)
{
	return NULL;
}

void dummy_func()
{
	pthread_t a_thread;

	pthread_create(&a_thread, NULL, thread_function, NULL);

	return;
}
