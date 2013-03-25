/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test the resulting attributes object (possibly modified by setting individual
 * attribute values) when used by pthread_create() defines the attributes of
 * the thread created.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Pass the newly created attribute object to pthread_create()
 * 3.  Test that thread is joinable, since using pthread_attr_init() will
 *     set the default detachstate to PTHREAD_CREATE_JOINABLE.
 * 4.  Pthread_detach() to test this.  It should
 *     not return errors since the thread should be joinable (non-detached).  If it
 *     returns an error, that means that the thread is not joinable, but rather
 *     in a detached state, and the test fails.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TIMEOUT 5		/* Timeout value of 5 seconds. */
#define INTHREAD 0		/* Control going to or is already for Thread */
#define INMAIN 1		/* Control going to or is already for Main */

int sem1;			/* Manual semaphore */

void *a_thread_func()
{

	/* Indicate to main() that the thread was created. */
	sem1 = INTHREAD;

	/* Wait for main to detach change the attribute object and try and detach this thread.
	 * Wait for a timeout value of 10 seconds before timing out if the thread was not able
	 * to be detached. */
	sleep(TIMEOUT);

	printf
	    ("Test FAILED: Did not detach the thread, main still waiting for it to end execution.\n");
	pthread_exit((void *)PTS_FAIL);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t new_attr;
	int ret_val;

	/* Initializing */
	sem1 = INMAIN;
	if (pthread_attr_init(&new_attr) != 0) {
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Create a new thread passing it the new attribute object */
	if (pthread_create(&new_th, &new_attr, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to indicate that the start routine for the thread has started. */
	while (sem1 == INMAIN)
		sleep(1);

	/* If pthread_detach fails, that means that the test fails as well. */
	ret_val = pthread_detach(new_th);

	if (ret_val != 0) {
		/* Thread is already detached. */
		if (ret_val == EINVAL) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		/* pthread_detach() failed for another reason. */
		else {
			printf("Error in pthread_detach(), error: %d\n",
			       ret_val);
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
