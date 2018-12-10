/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that pthread_rwlockattr_destroy()
 *    shall destroy a read write attributes object.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object using pthread_rwlockattr_init()
 * 2.  Destroy the attributes object using pthread_rwlockattr_destroy()
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_rwlockattr_t rwla;

	int rc;

	/* Initialize a rwlock attributes object */
	rc = pthread_rwlockattr_init(&rwla);
	if (rc != 0) {
		printf("Error at pthread_rwlockattr_init(), error code: %d\n",
		       rc);
		return PTS_UNRESOLVED;
	}

	/* Destroy the rwlock attributes object */
	rc = pthread_rwlockattr_destroy(&rwla);
	if (rc != 0) {
		printf
		    ("Error at pthread_rwlockattr_destroy(), error code: %d\n",
		     rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
