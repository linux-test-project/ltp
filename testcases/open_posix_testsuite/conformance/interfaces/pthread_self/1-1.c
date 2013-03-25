/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_self()
 *
 * Shall return the thread ID of the calling thread.  No errors are defined.
 *
 * Steps:
 * 1.  Create a new thread.
 * 2.  The function that the thread calls will call pthread_self() and store
 *     the return value (which is the thread ID of the thread calling it) into
 *     a global variable.
 * 3.  Call pthread_equal and verify that the thread IDs are the same.
 * 4.  Verify that the new thread ID is not the same as main thread ID.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

pthread_t new_th2;		/* Global thread to hold the value of when pthread_self
				   returns from the thread function. */

void *a_thread_func()
{
	new_th2 = pthread_self();
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th1;

	/* Create a new thread. */
	if (pthread_create(&new_th1, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to return */
	if (pthread_join(new_th1, NULL) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Call pthread_equal() and pass to it the new thread ID in both
	 * parameters.  It should return a non-zero value, indicating that
	 * both thread IDs are equal, and therefore refer to the same
	 * thread. */
	if (pthread_equal(new_th1, new_th2) == 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	if (pthread_equal(new_th1, pthread_self()) != 0) {
		printf("Test FAILED -- 2 threads have the same ID\n");
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
