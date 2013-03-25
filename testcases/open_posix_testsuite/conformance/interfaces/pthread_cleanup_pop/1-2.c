/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *  void pthread_cleanup_pop(int execution);
 *
 * Shall remove the routine at the top of the calling thread's cancelation cleanup stack and
 * optionally invoke it (if execute is non-zero).
 *
 * STEPS:
 * 1. Create a thread
 * 2. The thread will push a cleanup handler routine, then call pthread_cleanup_pop, setting
 *    'execution' to a zero value (meaning the cleanup handler shouldn't be executed)
 * 3. Verify that the cleanup handler was not called.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define CLEANUP_NOTCALLED 0
#define CLEANUP_CALLED 1

#define INTHREAD 0
#define INMAIN 1

int sem1;			/* Manual semaphore */
int cleanup_flag;

/* Cleanup handler */
void a_cleanup_func(void *flag_val)
{
	cleanup_flag = (long)flag_val;
	return;
}

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	pthread_cleanup_push(a_cleanup_func, (void *)CLEANUP_CALLED);
	pthread_cleanup_pop(0);

	/* Tell main that the thread has called the pop function */
	sem1 = INMAIN;

	/* Wait for main to say it's ok to continue (as it checks to make sure that
	 * the cleanup handler was called */
	while (sem1 == INMAIN)
		sleep(1);

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;

	/* Initializing values */
	sem1 = INTHREAD;
	cleanup_flag = CLEANUP_NOTCALLED;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to call the pthread_cleanup_pop */
	while (sem1 == INTHREAD)
		sleep(1);

	/* Check to verify that the cleanup handler was called */
	if (cleanup_flag == CLEANUP_CALLED) {
		printf("Test FAILED: Cleanup was incorrectly called\n");
		return PTS_FAIL;
	}

	/* Tell thread it can keep going now */
	sem1 = INTHREAD;

	/* Wait for thread to end execution */
	if (pthread_join(new_th, NULL) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
