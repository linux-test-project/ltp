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
 * 2. The thread will push 3 cleanup handler routines, then it will call the pop function 3
 *    times.
 * 3. Verify that the cleanup handlers are popped in order.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define CLEANUP_NOTCALLED 0
#define CLEANUP_CALLED 1

int cleanup_flag[3];		/* Array to hold the cleanup flags for the 3 cleanup handlers */
int i;

/* 3 Cleanup handlers */
void a_cleanup_func1(void *flag_val LTP_ATTRIBUTE_UNUSED)
{
	cleanup_flag[i] = 1;
	i++;
	return;
}

void a_cleanup_func2(void *flag_val LTP_ATTRIBUTE_UNUSED)
{
	cleanup_flag[i] = 2;
	i++;
	return;
}

void a_cleanup_func3(void *flag_val LTP_ATTRIBUTE_UNUSED)
{
	cleanup_flag[i] = 3;
	i++;
	return;
}

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	pthread_cleanup_push(a_cleanup_func1, NULL);
	pthread_cleanup_push(a_cleanup_func2, NULL);
	pthread_cleanup_push(a_cleanup_func3, NULL);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;

	/* Initializing values */
	for (i = 0; i < 3; i++)
		cleanup_flag[i] = 0;
	i = 0;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end execution */
	if (pthread_join(new_th, NULL) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Verify that the cancellation handlers are popped in order, that is:
	 * 3, 2, then 1. */
	if ((cleanup_flag[0] != 3) || (cleanup_flag[1] != 2)
	    || (cleanup_flag[2] != 1)) {
		printf
		    ("Test FAILED: Cleanup handlers not popped in order, expected 3,2,1, but got:\n");
		printf("%d, %d, %d\n", cleanup_flag[0], cleanup_flag[1],
		       cleanup_flag[2]);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
