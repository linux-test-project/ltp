/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_timedlock()
 *
 * It SHALL fail if:
 *
 * [EINVAL] - The process or thread would have blocked, and the abs_timeout parameter
 *	     specified in nano-seconds field value is less than 0 or greater than or equal
 * 	     to 1,000 million.
 *
 * Steps:
 *
 * 1. Create a thread.
 * 2. Call pthread_mutex_timedlock inside of the thread passing to it a negative number in the
 *    nano-seconds field of the 'abs_timeout'.
 * 3. Save the return value of pthread_mutex_timedlock().  It should be EINVAL.
 *
 */


#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_TIME -1		/* Invalid value of negative value
				   in the nano-seonds field of
				   'abs_timeout. */
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

	/* Check the return status of pthread_mutex_timedlock(). */
	if (ret != EINVAL) {
		printf("Test FAILED: Expected return code EINVAL, got: %d.\n",
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

	/* Lock the mutex */
	if (pthread_mutex_lock(&mutex) != 0) {
		perror("Error in pthread_mutex_lock()\n");
		pthread_exit((void *)PTS_UNRESOLVED);
		return (void *)PTS_UNRESOLVED;
	}

	/* Set nano-seconds to negative value. */
	timeout.tv_sec = time(NULL) + TIMEOUT;
	timeout.tv_nsec = INVALID_TIME;

	/* This should return EINVAL */
	ret = pthread_mutex_timedlock(&mutex, &timeout);

	/* Cleaning up the mutexes. */
	if (pthread_mutex_unlock(&mutex) != 0) {
		perror("Error in pthread_mutex_unlock().\n");
		pthread_exit((void *)PTS_UNRESOLVED);
		return (void *)PTS_UNRESOLVED;
	}
	if (pthread_mutex_destroy(&mutex) != 0) {
		perror("Error in pthread_mutex_destroy().\n");
		pthread_exit((void *)PTS_UNRESOLVED);
		return (void *)PTS_UNRESOLVED;
	}

	pthread_exit(0);
	return (void *)(0);
}
