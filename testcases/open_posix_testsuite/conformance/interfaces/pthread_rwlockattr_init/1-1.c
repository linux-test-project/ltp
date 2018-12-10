/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  adam.li REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlockattr_init()
 *   shall initialize a read-write attributes object 'attr' with the default
 *   value for all of the attributes defined by the implementation.

 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object with pthread_rwlockattr_init()
 * 2.  Call pthread_rwlockattr_getpshared() to check if the process-shared
 *     attribute is set as the default value PTHREAD_PROCESS_PRIVATE.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main(void)
{
	pthread_rwlockattr_t rwa;
	int rc;

#ifdef PTHREAD_PROCESS_SHARED
	int pshared;
#endif
	/* Initialize a read-write lock attributes object */
	rc = pthread_rwlockattr_init(&rwa);
	if (rc != 0) {
		printf("Test FAILED, pthread_rwlockattr_init() returns %d\n",
		       rc);
		return PTS_FAIL;
	}
#ifdef PTHREAD_PROCESS_SHARED
	/* If the symbol {PTHREAD_PROCESS_SHARED} is defined, the attribute
	 * process-shared should be provided and its default value should be
	 * PTHREAD_PROCESS_PRIVATE  */
	if (pthread_rwlockattr_getpshared(&rwa, &pshared) != 0) {
		printf("Error obtaining the attribute process-shared\n");
		return PTS_UNRESOLVED;
	}

	if (pshared == PTHREAD_PROCESS_PRIVATE) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf
		    ("Test FAILED, the default process-shared attribute is not PTHREAD_PROCESS_PRIVATE\n");
		return PTS_FAIL;
	}
#endif

	fprintf(stderr,
		"process-shared attribute is not available for testing\n");
	return PTS_UNSUPPORTED;
}
