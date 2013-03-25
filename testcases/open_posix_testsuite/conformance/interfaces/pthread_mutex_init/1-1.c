/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_init()
 *   initializes a mutex referenced by 'mutex' with attributes specified
 *   by 'attr'.  If 'attr' is NULL, the default mutex attributes are used.
 *   The effect shall be the same as passing the address of a default
 *   mutex attributes.

 * NOTE: There is no direct way to judge if two mutexes have the same effect,
 *       thus this test does not cover the statement in the last sentence.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	pthread_mutexattr_t mta;
	pthread_mutex_t mutex1, mutex2;
	int rc;

	/* Initialize a mutex attributes object */
	if ((rc = pthread_mutexattr_init(&mta)) != 0) {
		fprintf(stderr, "Error at pthread_mutexattr_init(), rc=%d\n",
			rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize mutex1 with the default mutex attributes */
	if ((rc = pthread_mutex_init(&mutex1, &mta)) != 0) {
		fprintf(stderr, "Fail to initialize mutex1, rc=%d\n", rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Initialize mutex2 with NULL attributes */
	if ((rc = pthread_mutex_init(&mutex2, NULL)) != 0) {
		fprintf(stderr, "Fail to initialize mutex2, rc=%d\n", rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
