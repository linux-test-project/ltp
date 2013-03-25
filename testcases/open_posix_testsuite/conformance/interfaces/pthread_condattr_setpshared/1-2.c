/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_condattr_setpshared()
 *
 *  It shall obtain the value of the process-shared attribute from 'attr'.
 *
 * Explanation:  To share a mutex between 2 processes, you need to map shared memory for
 * the mutex.  So whether the 'type' of the condattr is shared or private, it really will
 * not make a difference since both processes will always have access to the shared memory
 * as long as they the pointer to it.  So all we check here is that you can actually call
 * the pthread_condattr_setpshared() function, passing to it PTHREAD_PROCESS_SHARED and
 * PTHREAD_PROCESS_PRIVATE.
 *
 * Steps:
 * 1.  In a loop, initialize a pthread_condattr_t object with pthread_condattr_init()
 * 2.  Set 'pshared' of the object to PTHREAD_PROCESS_SHARED using pthread_condattr_setpshared
 * 3.  Call pthread_condattr_getpshared() to check if the process-shared
 *     attribute is set as PTHREAD_PROCESS_SHARED.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define NUM_OF_CONDATTR 10

int main(void)
{

	/* Make sure there is process-shared capability. */
#ifndef PTHREAD_PROCESS_SHARED
	fprintf(stderr,
		"process-shared attribute is not available for testing\n");
	return PTS_UNRESOLVED;
#endif

	pthread_condattr_t attr[NUM_OF_CONDATTR];
	int ret, i, pshared;

	for (i = 0; i < NUM_OF_CONDATTR; i++) {
		/* Initialize a cond attributes object */
		if (pthread_condattr_init(&attr[i]) != 0) {
			perror("Error at pthread_condattr_init()\n");
			return PTS_UNRESOLVED;
		}

		/* Set 'pshared' to PTHREAD_PROCESS_SHARED. */
		ret =
		    pthread_condattr_setpshared(&attr[i],
						PTHREAD_PROCESS_SHARED);
		if (ret != 0) {
			printf
			    ("Test FAILED: Could not set pshared to PTHREAD_PROCESS_SHARED, error: %d\n",
			     ret);
			return PTS_FAIL;
		}

		/* Get 'pshared'.  It should be PTHREAD_PROCESS_SHARED. */
		if (pthread_condattr_getpshared(&attr[i], &pshared) != 0) {
			printf
			    ("Test FAILED: obtaining the wrong process-shared attribute, expected PTHREAD_PROCESS_SHARED, but got: %d\n",
			     pshared);
			return PTS_FAIL;
		}

		if (pshared != PTHREAD_PROCESS_SHARED) {
			printf("Test FAILED: Incorrect pshared value: %d\n",
			       pshared);
			return PTS_FAIL;
		}

		/* Destory the cond attributes object */
		if (pthread_condattr_destroy(&attr[i]) != 0) {
			perror("Error at pthread_condattr_destroy()\n");
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
