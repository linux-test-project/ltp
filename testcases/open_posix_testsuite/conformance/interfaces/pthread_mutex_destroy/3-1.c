/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_destroy()
 * Upon successful completion, it shall return 0.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_mutex_t mutex;
	int rc;

	/* Initialize a mutex object */
	if ((rc = pthread_mutex_init(&mutex, NULL)) != 0) {
		fprintf(stderr, "Fail to initialize mutex, rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_mutex_destroy(&mutex)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* The following error codes are possible, but against assertion 5 */
	else if (rc == EBUSY) {
		fprintf(stderr,
			"Detected an attempt to destroy a mutex in use\n");
	} else if (rc == EINVAL) {
		fprintf(stderr, "The value specified by 'mutex' is invalid\n");
	}

	/* Any other returned value means the test failed */
	else {
		printf("Test FAILED (error: %i)\n", rc);
	}
	return PTS_FAIL;
}
