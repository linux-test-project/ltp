/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrierattr_init()
 *
 * The pthread_barrierattr_init() function shall initialize a barrier attributes
 * object attr with the default value for all of the attributes defined
 * by the implementation.
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

int main(void)
{
	int rc;
	pthread_barrierattr_t ba;
	int pshared;

	/* Initialize the barrier attribute object */
	rc = pthread_barrierattr_init(&ba);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error while initialize attribute object\n");
		return PTS_FAIL;
	}

	/* Get the pshared value of the initialized barrierattr object */
	if (pthread_barrierattr_getpshared(&ba, &pshared) != 0) {
		printf("Error at pthread_barrierattr_getpshared()\n");
		return PTS_UNRESOLVED;
	}

	/* The default should be PTHREAD_PROCESS_PRIVATE */
	if (pshared != PTHREAD_PROCESS_PRIVATE) {
		printf
		    ("Test FAILED: The process shared attribute was not set to "
		     "default value\n");
		return PTS_FAIL;
	}

	/* Cleanup */
	rc = pthread_barrierattr_destroy(&ba);
	if (rc != 0) {
		printf("Error at pthread_barrierattr_destroy() "
		       "return code: %d, %s", rc, strerror(rc));
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
