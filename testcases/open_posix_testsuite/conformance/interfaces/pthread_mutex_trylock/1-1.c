/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_trylock()
 *   is equivalent to pthread_mutex_lock() except that if the mutex object
 *   referenced by 'mutex' is currently locked (by any thread, including the
 *   current thread), the call shall return immediately.

 * Steps:
 *   -- Initilize a mutex object
 *   -- Create a secondary thread and have it lock the mutex
 *   -- Within the main thread, try to lock the mutex using
 	pthread_mutex_trylock() and EBUSY is expected
 *   -- Have the secondary thread unlock the mutex
 *   -- Within the main thread, try to lock the mutex again
 	and expect a successful locking.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

void *func(void *parm);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int t1_start = 0;
int t1_pause = 1;

int main(void)
{
	int i, rc;
	pthread_t t1;

	/* Create a secondary thread and wait until it has locked the mutex */
	pthread_create(&t1, NULL, func, NULL);
	while (!t1_start)
		sleep(1);

	/* Trylock the mutex and expect it returns EBUSY */
	rc = pthread_mutex_trylock(&mutex);
	if (rc != EBUSY) {
		fprintf(stderr, "Expected %d(EBUSY), got %d\n", EBUSY, rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Allow the secondary thread to go ahead */
	t1_pause = 0;

	/* Trylock the mutex for N times */
	for (i = 0; i < 5; i++) {
		rc = pthread_mutex_trylock(&mutex);
		if (rc == 0) {
			pthread_mutex_unlock(&mutex);
			break;
		} else if (rc == EBUSY) {
			sleep(1);
			continue;
		} else {
			fprintf(stderr,
				"Unexpected error code(%d) for pthread_mutex_lock()\n",
				rc);
			return PTS_UNRESOLVED;
		}
	}

	/* Clean up */
	pthread_join(t1, NULL);
	pthread_mutex_destroy(&mutex);

	if (i >= 5) {
		fprintf(stderr,
			"Have tried %d times but failed to get the mutex\n", i);
		return PTS_UNRESOLVED;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}

void *func(void *parm LTP_ATTRIBUTE_UNUSED)
{
	int rc;

	if ((rc = pthread_mutex_lock(&mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_lock(), rc=%d\n", rc);
		pthread_exit((void *)PTS_UNRESOLVED);
	}
	t1_start = 1;

	while (t1_pause)
		sleep(1);

	if ((rc = pthread_mutex_unlock(&mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_unlock(), rc=%d\n", rc);
		pthread_exit((void *)PTS_UNRESOLVED);
	}

	pthread_exit(0);
	return (void *)(0);
}
