/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_cancel
 * Upon successful completion will return a 0.
 *
 * STEPS:
 * 1. Create a thread
 * 2. Cancel that thread
 * 3. If pthread_cancel does not return [ESRCH] then it should return 0
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int sem;			/* Manual semaphore */
void *a_thread_func()
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* Indicate to main() that the thread has been created. */
	sem = 1;

	while (1)
		sleep(1);

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int ret;

	sem = 0;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure thread is created before we cancel it. */
	while (sem == 0)
		sleep(1);

	/* Send cancel request to thread */
	ret = pthread_cancel(new_th);

	if (ret != 0) {
		if (ret == ESRCH) {
			perror("Could not cancel thread\n");
			return PTS_UNRESOLVED;
		} else {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
