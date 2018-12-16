/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrier_init()
 *
 *
 * The pthread_barrier_init() function shall allocate any resources
 * required to use the barrier referenced by barrier and shall initialize
 * the barrier with attributes referenced by attr. If attr is NULL,
 * the default barrier attributes shall be used;
 * the effect is the same as passing the address of a default barrier attributes object.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define COUNT 1

static pthread_barrier_t barrier;

int main(void)
{
	int rc;
	pthread_barrierattr_t ba;

	/* Intilized barrier with NULL attribute, check that this can be done. */
	rc = pthread_barrier_init(&barrier, NULL, COUNT);

	if (rc != 0) {
		printf("Test FAILED: Error at pthread_barrier_init() "
		       "return code %d, %s\n", rc, strerror(rc));
		return PTS_FAIL;
	}

	/* Cleanup */
	if (pthread_barrier_destroy(&barrier) != 0) {
		printf("Error at pthread_barrier_destroy() "
		       " return code: %d, %s\n", rc, strerror(rc));
		return PTS_UNRESOLVED;
	}

	/* Initialize a barrier attribute object */
	if (pthread_barrierattr_init(&ba) != 0) {
		printf("Error at pthread_barrierattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Initialize barrier with this barrier attribute object */
	rc = pthread_barrier_init(&barrier, &ba, COUNT);
	if (rc != 0) {
		printf("Test FAILED: Error at 2nd pthread_barrier_init() "
		       "return code %d, %s\n", rc, strerror(rc));
		return PTS_FAIL;
	}

	/* Cleanup */
	if (pthread_barrierattr_destroy(&ba) != 0) {
		printf("Error at pthread_barrierattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_barrier_destroy(&barrier) != 0) {
		printf("Error at pthread_barrier_destroy() "
		       " return code: %d, %s\n", rc, strerror(rc));
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
