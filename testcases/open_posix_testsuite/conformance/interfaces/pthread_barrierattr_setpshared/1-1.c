/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrierattr_setpshared()
 *
 * The pthread_barrierattr_setpshared() function shall
 * set the process-shared attribute in an initialized attributes object referenced by attr.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{

	/* Make sure there is process-shared capability. */
#ifndef PTHREAD_PROCESS_SHARED
	fprintf(stderr,
		"process-shared attribute is not available for testing\n");
	return PTS_UNSUPPORTED;
#endif

	pthread_barrierattr_t ba;
	int pshared = PTHREAD_PROCESS_SHARED;
	int pshared2 = PTHREAD_PROCESS_PRIVATE;
	int rc;

	/* Initialize a barrier attributes object */
	if (pthread_barrierattr_init(&ba) != 0) {
		printf("Error at pthread_barrierattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set pshared to PTHREAD_PROCESS_SHARED */
	rc = pthread_barrierattr_setpshared(&ba, pshared);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error at pthread_barrierattr_setpshared()\n");
		return PTS_FAIL;
	}

	if (pthread_barrierattr_getpshared(&ba, &pshared) != 0) {
		printf("Error at pthread_barrierattr_getpshared()\n");
		return PTS_FAIL;
	}

	if (pshared != PTHREAD_PROCESS_SHARED) {
		printf("Test FAILED: Got error shared attribute value %d\n",
		       pshared);
		return PTS_FAIL;
	}

	/* Set pshared to PTHREAD_PROCESS_SHARED */
	rc = pthread_barrierattr_setpshared(&ba, pshared2);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error at pthread_barrierattr_setpshared()\n");
		return PTS_FAIL;
	}

	if (pthread_barrierattr_getpshared(&ba, &pshared2) != 0) {
		printf("Error at pthread_barrierattr_getpshared()\n");
		return PTS_FAIL;
	}

	if (pshared2 != PTHREAD_PROCESS_PRIVATE) {
		printf("Test FAILED: Got error shared attribute value %d\n",
		       pshared);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
