/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_condattr_destroy()
 *   Upon successful completion, pthread_condattr_destroy() shall
 *   return a value of 0.
 *
 * Steps:
 * 1.  Initialize a pthread_condattr_t object using pthread_condattr_init()
 * 2.  Destroy that initialized attribute using pthread_condattr_destroy().
 *     This should return 0;
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_condattr_t condattr;

	/* Initialize a condition variable attributes object */
	if (pthread_condattr_init(&condattr) != 0) {
		fprintf(stderr,
			"Cannot initialize condition variable attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the condition variable attributes object */
	if (pthread_condattr_destroy(&condattr) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}
