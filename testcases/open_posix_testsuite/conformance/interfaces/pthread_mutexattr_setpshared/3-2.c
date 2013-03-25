/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_setpshared()
 *
 * It MAY fail if:
 *
 * [EINVAL] - the new value pshared value is outside the range of legal values for that
 *            attribute.
 *
 * Steps:
 *
 * 1. Pass to pthread_mutexattr_setpshared() a negative value in the 'pshared' parameter..
 * 2. It may return the value of EINVAL.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_PSHARED_VALUE -1

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

	/* Set the attribute to INVALID_PSHARED_VALUE.  */
	ret = pthread_mutexattr_setpshared(&mta, INVALID_PSHARED_VALUE);
	if (ret != 0) {
		if (ret == EINVAL) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf("Test FAILED: Expected return code 0 or EINVAL, got: %d",
		       ret);
		return PTS_FAIL;
	}

	printf
	    ("Test PASSED: NOTE*: Returned 0 on error, though standard states 'may' fail.\n");
	return PTS_PASS;

}
