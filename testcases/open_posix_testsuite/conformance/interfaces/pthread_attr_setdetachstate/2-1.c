/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_attr_setdetachstate()
 *
 * If the thread is created detached, then use of the ID of the newly created
 * thread by the pthread_detach() or pthread_join() function is an error.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init().
 * 2.  Using pthread_attr_setdetachstate(), set the detachstate to
 *     PTHREAD_CREATE_DETACHED.
 * 3.  Create a thread with this attribute.
 * 4.  Call pthread_detach() and pthread_join() on this thread, it should give
 *     an error.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

void *a_thread_func()
{

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t new_attr;
	int ret_val;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0) {
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Set the attribute object to PTHREAD_CREATE_DETACHED. */
	if (pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED) !=
	    0) {
		perror("Error in pthread_attr_setdetachstate()\n");
		return PTS_UNRESOLVED;
	}

	/* Create a new thread passing it the new attribute object */
	if (pthread_create(&new_th, &new_attr, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* If pthread_join() or pthread_detach fail, that means that the
	 * test fails as well. */
	ret_val = pthread_join(new_th, NULL);

	if (ret_val != EINVAL) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	ret_val = pthread_detach(new_th);

	if (ret_val != EINVAL) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
