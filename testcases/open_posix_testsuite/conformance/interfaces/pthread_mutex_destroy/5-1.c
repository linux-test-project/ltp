/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_destroy()
 * 	It shall be safe to destroy an initialized mutex that is unlocked.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

pthread_mutex_t mutex;

int main(void)
{
	int rc;

	/* Initialize mutex with the default mutex attributes */
	if ((rc = pthread_mutex_init(&mutex, NULL)) != 0) {
		fprintf(stderr, "Fail to initialize mutex, rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}

	/* Lock mutex */
	if ((rc = pthread_mutex_lock(&mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_lock(), rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}
	sleep(1);
	/* Unlock */
	if ((rc = pthread_mutex_unlock(&mutex)) != 0) {
		fprintf(stderr, "Error at pthread_mutex_unlock(), rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}
	/* Destroy mutex after it is unlocked */
	if ((rc = pthread_mutex_destroy(&mutex)) != 0) {
		fprintf(stderr,
			"Fail to destroy mutex after being unlocked, rc=%d\n",
			rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
