/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_unlock()
 *   shall release the mutex object 'mutex'.

 * Steps:
 *   -- Initilize a mutex object
 *   -- Get the mutex using pthread_mutex_lock()
 *   -- Release the mutex using pthread_mutex_unlock()
 *   -- Try to get the mutex using pthread_mutex_trylock()
 *   -- Release the mutex using pthread_mutex_unlock()
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void)
{
	int rc;

	/* Get the mutex using pthread_mutex_lock() */
	if ((rc = pthread_mutex_lock(&mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_lock(), rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}

	/* Release the mutex using pthread_mutex_unlock() */
	if ((rc = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Get the mutex using pthread_mutex_trylock() */
	if ((rc = pthread_mutex_trylock(&mutex)) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Release the mutex using pthread_mutex_unlock() */
	if ((rc = pthread_mutex_unlock(&mutex)) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
