/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_once()
 *
 *  Dynamic package installation.  Tihe first call to pthread_once() by any
 *  thread in a process, with a given 'once_control', shall call the
 *  'init_routine' with no arguments.  Subsequent calls of pthread_once()
 *  with the same once_control shall not call the 'init_routine'.  The
 *  'once_control' paramter shall determine whether the associated
 *  initialization routine has been called.
 *
 * STEPS:
 * 1.Initialize a pthread_once object
 * 2.Call pthread_once passing it this object
 * 3.Call pthread_once again, this time, it shouldn't execute the function
 *   passed to it.  If it does, the test fails.
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

/* Keeps track of how many times the init function has been called. */
static int init_flag;

/* The init function that pthread_once calls */
void an_init_func(void)
{
	init_flag++;
}

int main(void)
{
	int ret;

	pthread_once_t once_control = PTHREAD_ONCE_INIT;

	init_flag = 0;

	/* Call pthread_once, passing it the once_control */
	ret = pthread_once(&once_control, an_init_func);
	if (ret != 0) {
		printf("pthread_once failed\n");
		return PTS_UNRESOLVED;
	}
	/* Call pthread_once again. The init function should not be
	 * called. */
	ret = pthread_once(&once_control, an_init_func);
	if (ret != 0) {
		printf("pthread_once failed\n");
		return PTS_UNRESOLVED;
	}

	if (init_flag != 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
