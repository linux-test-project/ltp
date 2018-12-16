/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_rwlockattr_destroy()
 *   A destroyed 'attr' attributes object can be reinitialized using
 *   pthread_rwlockattr_init()
 *
 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object using pthread_rwlockattr_init()
 * 2.  Destroy that initialized attribute using pthread_rwlockattr_destroy()
 * 3.  Initialize the pthread_rwlockattr_t object again. This should not result
 *     in any error.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_rwlockattr_t rwla;
	int rc = 0;

	/* Initialize a rwlock attributes object */
	rc = pthread_rwlockattr_init(&rwla);
	if (rc != 0) {
		printf("Cannot initialize rwlock attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the rwlock attributes object */
	rc = pthread_rwlockattr_destroy(&rwla);
	if (rc != 0) {
		printf
		    ("Cannot destroy the rwlock attributes object, error code: %d\n",
		     rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize the rwlock attributes object again.  This shouldn't result in an error. */
	rc = pthread_rwlockattr_init(&rwla);

	if (rc != 0) {
		printf("Test FAILED, with error: %d\n", rc);
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
