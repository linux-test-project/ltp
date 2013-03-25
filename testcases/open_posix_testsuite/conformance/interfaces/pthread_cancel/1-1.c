/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_cancel
 * Shall request that 'thread' shall be canceled.  The target thread's
 * cancelability state and type determines when the cancelation takes
 * effect.
 *
 * Test when a thread is PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_ASYNCHRONOUS
 *
 * STEPS:
 * 1. Create a thread.
 * 2. In the thread function, set the set and type to
 *    PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_ASYNCHRONOUS
 * 3. Setup a cleanup handler for the thread.
 * 4. Send out a thread cancel request to the new thread
 * 5. If the cancel request was honored immediately and correctly, the
 *    cleanup handler would have been executed, and the test will pass.
 * 6. If not, the thread will wait for 10 seconds before it exits itself,
 *    and the test will fail.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define INTHREAD 0		/* Control going to or is already for Thread */
#define INMAIN 1		/* Control going to or is already for Main */

int sem1;			/* Manual semaphore */
int cleanup_flag;

/* Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is -1, it means that the thread was canceled. */
void a_cleanup_func()
{
	cleanup_flag = 1;
	return;
}

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	pthread_cleanup_push(a_cleanup_func, NULL);

	/* Indicate to main() that the thread has been created. */
	sem1 = INMAIN;

	/* Wait until main() has sent out a cancel request, meaning until it
	 * sets sem1==0 */
	while (sem1 == 1)
		sleep(1);

	/* Give thread 10 seconds to time out.  If the cancel request was not
	 * honored until now, the test will fail. */
	sleep(10);

	/* Shouldn't get here if the cancel request was honored immediately
	 * like it should have been. */
	cleanup_flag = -1;
	pthread_cleanup_pop(0);
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;

	/* Initializing values */
	sem1 = INTHREAD;
	cleanup_flag = 0;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure thread is created before we cancel it. (wait for
	 * a_thread_func() to set sem1=1.) */
	while (sem1 == INTHREAD)
		sleep(1);

	if (pthread_cancel(new_th) != 0) {
		printf("Test FAILED: Couldn't cancel thread\n");
		return PTS_FAIL;
	}

	/* Indicate to the thread function that the thread cancel request
	 * has been sent to it. */
	sem1 = INTHREAD;

	/* Wait for thread to return. (i.e. either canceled correctly and
	 * called the cleanup function, or timed out after 10 seconds. */
	while (cleanup_flag == INTHREAD)
		sleep(1);

	/* This means that the cleanup function wasn't called, so the cancel
	 * request was not honord immediately like it should have been. */
	if (cleanup_flag < 0) {
		printf("Test FAILED: Cancel request timed out\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
