/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlockattr_setpshared()
 *
 *  It shall set the value of the process-shared attribute from 'attr'.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object with pthread_rwlockattr_init().
 * 2.  Set the process shared to PTHREAD_PROCESS_PRIVATE.
 * 3.  Get the attribute and test whether the setting in step 2 is correct.
 * 4.  Set the process shared to PTHREAD_PROCESS_SHARED.
 * 5.  Get the attribute and test whether the setting in step 4 is correct.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	pthread_rwlockattr_t rwla;
	int pshared;
	int rc = 0;

#ifndef PTHREAD_PROCESS_SHARED
	printf("process-shared attribute is not available for testing\n");
	return PTS_UNSUPPORTED;
#endif

	/* Initialize a rwlock attributes object */
	if (pthread_rwlockattr_init(&rwla) != 0) {
		printf("Error at pthread_rwlockattr_init()\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_rwlockattr_setpshared(&rwla, PTHREAD_PROCESS_PRIVATE);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error at pthread_rwlockattr_setpshared(), return error: %d\n",
		     rc);
		return PTS_FAIL;
	}

	if (pthread_rwlockattr_getpshared(&rwla, &pshared) != 0) {
		printf("Error at pthread_rwlockattr_getpshared()\n");
		return PTS_UNRESOLVED;
	}

	if (pshared != PTHREAD_PROCESS_PRIVATE) {
		printf
		    ("Test FAILED: Expect PTHREAD_PROCESS_PRIVATE, but got %d\n",
		     pshared);
		return PTS_FAIL;
	}

	rc = pthread_rwlockattr_setpshared(&rwla, PTHREAD_PROCESS_SHARED);
	if (rc != 0) {
		printf
		    ("Test FAILED: Error at pthread_rwlockattr_setpshared(), return error: %d\n",
		     rc);
		return PTS_FAIL;
	}

	if (pthread_rwlockattr_getpshared(&rwla, &pshared) != 0) {
		printf("Error at pthread_rwlockattr_getpshared()\n");
		return PTS_UNRESOLVED;
	}

	if (pshared != PTHREAD_PROCESS_SHARED) {
		printf
		    ("Test FAILED: Expect PTHREAD_PROCESS_SHARED, but got %d\n",
		     pshared);
		return PTS_FAIL;
	}

	if (pthread_rwlockattr_destroy(&rwla) != 0) {
		printf("Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
