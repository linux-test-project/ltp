/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_setpshared()
 *
 * Upon success, it returns 0.
 *
 * Steps:
 *
 * 1. Initialized a mutexattr object.
 * 2. Set the attributes object to PTHREAD_PROCESS_PRIVATE. The error return code should be 0.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{

	/* Make sure there is process-shared capability. */
#ifndef PTHREAD_PROCESS_SHARED
	fprintf(stderr,
		"process-shared attribute is not available for testing\n");
	return PTS_UNRESOLVED;
#endif

	pthread_mutexattr_t mta;
	int ret;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set the attribute to PTHREAD_PROCESS_PRIVATE.  */
	ret = pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_PRIVATE);
	if (ret != 0) {
		printf("Test FAILED: Expected return code 0, got: %d", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
