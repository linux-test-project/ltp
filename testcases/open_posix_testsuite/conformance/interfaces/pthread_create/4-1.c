/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *  Test upon successful completion, pthread_create() shall store the ID of the
 *  the created thread in the location referenced by 'thread'.
 *
 * Steps:
 * 1.  Create a thread using pthread_create()
 * 2.  Save the thread ID resulting from pthread_create()
 * 3.  Get the thread ID from the new thread by calling
 *     pthread_self().
 * 4.  These 2 values should be equal. (i.e. the one from pthread_create()
 *     and the one from pthread_self()).
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "posixtest.h"

static void *a_thread_func();
static pthread_t self_th;

int main(void)
{
	pthread_t new_th;
	int ret;

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	ret = pthread_join(new_th, NULL);
	if (ret) {
		fprintf(stderr, "pthread_join(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	/* If the value of pthread_self() and the return value from
	 * pthread_create() is equal, then the test passes. */
	if (pthread_equal(new_th, self_th) == 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

static void *a_thread_func()
{
	self_th = pthread_self();
	return NULL;
}
