/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_destroy()
 *   Upon succesful completion, it shall return a 0
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_cond_t cond;
	int rc;

	/* Initialize a cond object */
	if ((rc = pthread_cond_init(&cond, NULL)) != 0) {
		fprintf(stderr, "Fail to initialize cond, rc=%d\n", rc);
		return PTS_UNRESOLVED;
	}

	if ((rc = pthread_cond_destroy(&cond)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* Check if returned values are tolerable */
	else if (rc == EBUSY) {
		fprintf(stderr,
			"Detected an attempt to destroy a cond in use\n");
		return PTS_FAIL;
	} else if (rc == EINVAL) {
		fprintf(stderr, "The value specified by 'cond' is invalid\n");
		return PTS_FAIL;
	}

	/* Any other returned value means the test failed */
	else {
		printf("Test FAILED (error %i unexpected)\n", rc);
		return PTS_FAIL;
	}
}
