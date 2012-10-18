/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_create() creates a new thread with attributes specified
 * by 'attr', within a process.
 *
 * Steps:
 * 1.  Create a thread using pthread_create()
 * 2.  Cancel that thread with pthread_cancel()
 * 3.  If that thread doesn't exist, then it pthread_cancel() will return
 *     an error code.  This would mean that pthread_create() did not create
 *     a thread successfully.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

void *a_thread_func()
{
	sleep(10);

	/* Shouldn't reach here.  If we do, then the pthread_cancel()
	 * function did not succeed. */
	perror("Could not send cancel request correctly\n");
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) < 0)
	{
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Try to cancel the newly created thread.  If an error is returned,
	 * then the thread wasn't created successfully. */
	if (pthread_cancel(new_th) != 0)
	{
		printf("Test FAILED: A new thread wasn't created\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
