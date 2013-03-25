/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_getpshared()
 *
 * It MAY fail if:
 *
 * [EINVAL] - 'attr' is invalid.
 *
 * Steps:
 * 1.  Pass to pthread_mutexattr_getpshared() an uninitialized and invalid 'attr' object.
 * 2.  The error code returned may be EINVAL.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
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

	/* Pass the uninitialized and invalid 'attr' object. */
	memset(&mta, 0, sizeof(mta));

	/* The default 'pshared' attribute should be PTHREAD_PROCESS_PRIVATE  */
	ret = pthread_mutexattr_getpshared(&mta, &pshared);
	if (ret != 0) {
		if (ret == EINVAL) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf
		    ("Test FAILED: Incorrect return code. Expected 0 or EINVAL, got: %d\n",
		     ret);
		return PTS_FAIL;

	}

	printf
	    ("Test PASSED: NOTE*: Returned 0 on error, though standard states 'may' fail.\n");
	return PTS_PASS;
}
