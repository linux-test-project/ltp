/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_create()
 *
 * If success, pthread_create() returns zero.
 *
 * Steps:
 * 1.  Create a thread using pthread_create(), passing to it all valid values.
 * 2.  If the return code was not EGAIN, EPERM or EINVAL, it should return 0.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

/* Thread starting routine that really does nothing. */
void *a_thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	int ret;

	/* Create new thread and check the return value. */
	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret != 0)
	{
		if ((ret != EINVAL) && (ret != EAGAIN) && (ret != EPERM))

		printf("Test FAILED: Wrong return code: %d\n", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
