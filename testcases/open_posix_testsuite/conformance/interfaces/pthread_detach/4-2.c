/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_detach()
 *
 * Upon failure, it shall return an error number:
 * -[EINVAL] The implemenation has detected that the value specified by
 * 'thread' does not refer to a joinable thread.
 * -[ESRCH] No thread could be found corresponding to that thread

 * It shall not return an error code of [EINTR]
 *
 * STEPS:
 * 1. Create a thread.
 * 2.Wait 'till the thread exits.
 * 3.Try and detach this thread.
 * 4.Check the return value and make sure it is ESRCH
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

/* Thread function */
void *a_thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int ret;

	/* Create the thread */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait 'till the thread returns.
	 * The thread could have ended by the time we try to join, so
	 * don't worry about it, just so long as other errors don't
	 * occur. The point is to make sure the thread has ended execution. */
	if (pthread_join(new_th, NULL) == EDEADLK) {
		perror("Error joining thread\n");
		return PTS_UNRESOLVED;
	}

	/* Detach the non-existant thread. */
	ret = pthread_detach(new_th);

	/* Check return value of pthread_detach() */
	if (ret != ESRCH) {
		printf
		    ("Test FAILED: Incorrect return code: %d instead of ESRCH\n",
		     ret);
		return PTS_FAIL;

	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
