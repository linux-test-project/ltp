/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_timedlock()
 *
 * It shall fail if:
 *
 * [ETIMEDOUT] - The timeout expried and the mutex could not be locked.
 *
 * Steps:
 *
 * 1. In main(), lock the mutex then create a thread.
 * 2. Inside the thread, call pthread_mutex_timedlock.  It should block and return
 *    ETIMEDOUT.
 * 3. Save the return value of pthread_mutex_timedlock() and cleanup mutex stuff.
 * 4. Anaylse the return value.
 *
 */


#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT 3		/* 3 seconds of timeout time for
				   pthread_mutex_timedlock(). */
static void *f1(void *parm);

static int ret;			/* Save return value of
				   pthread_mutex_timedlock(). */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	/* The mutex */

/****************************
 *
 * MAIN()
 *
 * *************************/
int main(void)
{
	pthread_t new_th;

	/* Lock the mutex before creating the thread. */
	if (pthread_mutex_lock(&mutex) != 0) {
		perror("Error in pthread_mutex_lock in main().\n");
		return PTS_UNRESOLVED;
	}

	/* Create a thread that will call pthread_mutex_timedlock */
	if (pthread_create(&new_th, NULL, f1, NULL) != 0) {
		perror("Error in pthread_create().\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end. */
	if (pthread_join(new_th, NULL) != 0) {
		perror("Error in pthread_join().\n");
		return PTS_UNRESOLVED;
	}

	/* Cleaning up the mutexes. */
	if (pthread_mutex_unlock(&mutex) != 0) {
		perror("Error in pthread_mutex_unlock().\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_mutex_destroy(&mutex) != 0) {
		perror("Error in pthread_mutex_destroy().\n");
		return PTS_UNRESOLVED;
	}

	/* Check the return status of pthread_mutex_timedlock(). */
	if (ret != ETIMEDOUT) {
		printf
		    ("Test FAILED: Expected return code ETIMEDOUT, got: %d.\n",
		     ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

/****************************
 *
 * Thread's start routine.
 * f1()
 *
 * *************************/
static void *f1(void *parm PTS_ATTRIBUTE_UNUSED)
{
	struct timespec timeout;

	timeout.tv_sec = time(NULL);
	timeout.tv_nsec = 0;

	/* This should block since the mutex is not owned by anyone right now.
	 * Save the return value. */
	ret = pthread_mutex_timedlock(&mutex, &timeout);

	pthread_exit(0);
	return (void *)(0);
}
