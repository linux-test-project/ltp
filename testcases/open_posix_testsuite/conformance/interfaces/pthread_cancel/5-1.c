/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_cancel
 * It may return an error number of:
 * -[ESRCH] No thread could be found corresponding to that thread ID

 * It shall not return an error code of [EINTR]
*
 * STEPS:
 * 1. Create a thread
 * 2. Wait 'till the thread has ended execution
 * 3. Send a cancel request to the thread (which isn't existing anymore)
 * 3. It should return an error code of [ESRCH], else test fails.
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
	int ret;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to finish execution. */
	pthread_join(new_th, NULL);

	/* Try and cancel thread.  It should return an error because it
	 * already is canceled and doesn't exist anymore.  */
	ret = pthread_cancel(new_th);

	if (ret != 0) {
		if (ret == ESRCH) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf("Test FAILED: Returned error code other than [ESRCH]\n");
		return PTS_FAIL;
	}

	printf
	    ("Test PASSED: *NOTE: Returned 0 on error, though standard states 'may' fail.\n");
	return PTS_PASS;

}
