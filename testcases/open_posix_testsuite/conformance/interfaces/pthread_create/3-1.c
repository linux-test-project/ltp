/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test if the attributes specified by 'attr' are modified later, the thread's
 *  attributes shall not be affected.
 *  The attribute that will be tested is the detached state.
 *
 *
 * Steps:
 * 1.  Set a pthread_attr_t object to be PTHREAD_CREATE_JOINABLE.
 * 2.  Create a new thread using pthread_create() and passing this attribute
 *     object.
 * 3.  Change the attribute object to be in a detached state rather than
 *     joinable.
 * 4.  Doing this should not effect the fact that the thread that was created
 *     is joinable, and so calling the functions pthread_detach() should not fail.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMEOUT 10

static volatile int sem;

void *a_thread_func()
{
	sem = 1;

	/* Wait for main to detach change the attribute object and try and detach this thread.
	 * Wait for a timeout value of 10 seconds before timing out if the thread was not able
	 * to be detached. */
	sleep(TIMEOUT);

	/* We should not get here */
	exit(PTS_FAIL);
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t attr;
	int ret;

	if ((ret = pthread_attr_init(&attr))) {
		fprintf(stderr, "pthread_attr_init(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;

	}

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (ret) {
		fprintf(stderr,
		        "pthread_attr_setdetachstate(..., PTHREAD_CREATE_JOINABLE): %s\n",
		        strerror(ret));
		return PTS_UNRESOLVED;
	}

	ret = pthread_create(&new_th, &attr, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	struct timespec thread_create_ts = {0, 10000000};
	while (!sem)
		nanosleep(&thread_create_ts, NULL);

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret) {
		fprintf(stderr,
		        "pthread_attr_setdetachstate(..., PTHREAD_CREATE_JOINABLE): %s\n",
		        strerror(ret));
		return PTS_UNRESOLVED;
	}

	/* The new thread should still be able to be detached. */
	ret = pthread_detach(new_th);
	if (ret) {
		fprintf(stderr,
		        "pthread_detach() failed on joinable thread: %s\n",
			strerror(ret));
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
