/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_cancel()
 *
 * Any destructors for thread_specific data will be called after
 * all cleanup handlers return
 *
 * Steps:
 * 1.  Create a new thread.
 * 2.  Create a thread specific object in the thread with a destructor
 * 3.  Add a cleanup function in the thread
 * 4.  Call pthread_cancel on the thread.
 * 5.  Make sure that the destructor was called after the cleanup handler
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "2-3"
#define FUNCTION "pthread_cancel"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int cleanup_flag = 0;
int destructor_flag = 0;
int sem = 0;			/* manual semaphore */
struct timespec destructor_time, cleanup_time;

/*
   Destructor for the Thread Specific Data
 */
void destructor(void *tmp LTP_ATTRIBUTE_UNUSED)
{
	clock_gettime(CLOCK_REALTIME, &destructor_time);
	destructor_flag = 1;
}

/*
   Cleanup Handler for the Thread
 */
void cleanup_function()
{
	clock_gettime(CLOCK_REALTIME, &cleanup_time);
	cleanup_flag = 1;
}

/* Thread's function. */
void *a_thread_func(void *tmp LTP_ATTRIBUTE_UNUSED)
{
	pthread_key_t key;
	int value = 1;
	int rc = 0;

	/* To enable thread immediate cancelation, since the default
	 * is PTHREAD_CANCEL_DEFERRED. */
	rc = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setcanceltype\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_key_create(&key, destructor);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_key_create\n");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_setspecific(key, &value);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_setspecific\n");
		exit(PTS_UNRESOLVED);
	}

	pthread_cleanup_push(cleanup_function, NULL);

	/* Tell main that the key is created */
	sem = 1;

	/* Sleep forever */
	while (1)
		sleep(5);

	pthread_cleanup_pop(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int rc = 0;
	double diff;
	sem = 0;

	/* Create a new thread. */
	rc = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		exit(PTS_UNRESOLVED);
	}

	/* Wait for the thread to be ready */
	while (sem == 0)
		sleep(1);

	/* Cancel the thread. */
	rc = pthread_cancel(new_th);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_cancel\n");
		exit(PTS_UNRESOLVED);
	}

	/* Delay enough so that the destructor must have been called */
	sleep(5);

	if (cleanup_flag != 1) {
		printf(ERROR_PREFIX
		       "Test FAIL: Cleanup handler was not executed.\n");
		exit(PTS_FAIL);
	}

	if (destructor_flag != 1) {
		printf(ERROR_PREFIX
		       "Test FAIL: Destructor was not executed.\n");
		exit(PTS_FAIL);
	}

	diff = destructor_time.tv_sec - cleanup_time.tv_sec;
	diff +=
	    (double)(destructor_time.tv_nsec -
		     cleanup_time.tv_nsec) / 1000000000.0;
	if (diff < 0) {
		printf(ERROR_PREFIX
		       "Test FAIL: Destructor called before Cleanup Handler\n");
		exit(PTS_FAIL);
	}

	printf("Test PASSED\n");
	exit(PTS_PASS);
}
