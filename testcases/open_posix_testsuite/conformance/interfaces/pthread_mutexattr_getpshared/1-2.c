/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_getpshared()
 *
 *  It shall obtain the value of the process-shared attribute from 'attr'.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Set 'pshared' of the object to PTHREAD_PROCESS_SHARED.
 * 3.  Call pthread_mutexattr_getpshared() to check if the process-shared
 *     attribute is set as PTHREAD_PROCESS_SHARED.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
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
	int pshared;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set 'pshared' to PTHREAD_PROCESS_SHARED. */
	ret = pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_SHARED);
	if (ret != 0) {
		printf("Error in pthread_mutexattr_setpshared(), error: %d\n",
		       ret);
		return PTS_UNRESOLVED;
	}

	/* Get 'pshared'.  */
	if (pthread_mutexattr_getpshared(&mta, &pshared) != 0) {
		fprintf(stderr,
			"Error obtaining the attribute process-shared\n");
		return PTS_UNRESOLVED;
	}

	if (pshared != PTHREAD_PROCESS_SHARED) {
		printf("Test FAILED: Incorrect pshared value: %d\n", pshared);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
