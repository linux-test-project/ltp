  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.

   Test this function is defined:

   int pthread_equal(pthread_t, pthread_t);
   */

#include <pthread.h>

pthread_t a, b;
int tmp;

void dummy_func()
{
	tmp = pthread_equal(a, b);
	return;
}
