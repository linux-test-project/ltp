/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_timedlock()
 *
 * Upon success, it returns 0.
 *
 * Steps:
 *
 * 1. Create a thread, and call pthread_mutex_timedlock inside of it.  It should not block
 *    and should return 0 since it will be the only one owning the mutex.
 * 2. Save the return value of pthread_mutex_timedlock().  It should be 0.
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
void *f1(void *parm);

int ret;			/* Save return value of
				   pthread_mutex_timedlock(). */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	/* The mutex */
time_t currsec1, currsec2;	/* Variables for saving time before
				   and afer locking the mutex using
				   pthread_mutex_timedlock(). */
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
	if (ret != 0) {
		printf("Test FAILED: Expected return code 0, got: %d.\n", ret);
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
void *f1(void *parm LTP_ATTRIBUTE_UNUSED)
{
	struct timespec timeout;

	timeout.tv_sec = time(NULL) + TIMEOUT;
	timeout.tv_nsec = 0;

	/* This should not block since the mutex is not owned by anyone right now.
	 * Save the return value. */
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
