/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_init()
 *   Upon succesful completion, it shall return a 0
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_mutexattr_t mta;
	pthread_mutex_t mutex;
	int rc;

	/* Initialize a mutex attributes object */
	if ((rc = pthread_mutexattr_init(&mta)) != 0) {
		fprintf(stderr, "Error at pthread_mutexattr_init(), rc=%d\n",
			rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize a mutex object with the default mutex attributes */
	if ((rc = pthread_mutex_init(&mutex, &mta)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* Check if returned values are tolerable */
	else if (rc == ENOMEM) {
		fprintf(stderr,
			"Insufficient memory to initialize the mutex\n");
		return PTS_UNRESOLVED;
	} else if (rc == EAGAIN) {
		fprintf(stderr,
			"Lack of the necessary resources to initilize the mutex\n");
		return PTS_UNRESOLVED;
	} else if (rc == EPERM) {
		fprintf(stderr, "Permission denied\n");
		return PTS_UNRESOLVED;
	} else if (rc == EBUSY) {
		fprintf(stderr,
			"Detected an attemp to reinitilize a previously initilized mutex\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
