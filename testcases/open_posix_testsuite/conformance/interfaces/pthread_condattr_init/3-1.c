/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_condattr_init()
 *   Upon successful completion, pthread_condattr_init() shall return a value of 0.

 * Steps:
 * 1.  Initialize a pthread_condattr_t object with pthread_condattr_init()
 * 2.  ENOMEM is the only error it returns, so if it doesn't return that error,
 *     the return number should be 0.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_condattr_t condattr;
	int rc;

	/* Initialize a condition variable attributes object */
	if ((rc = pthread_condattr_init(&condattr)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* Insufficient memory exists to initialize the condition variable attributes object */
	else if (rc == ENOMEM) {
		fprintf(stderr, "pthread_condattr_init() returns ENOMEM\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
