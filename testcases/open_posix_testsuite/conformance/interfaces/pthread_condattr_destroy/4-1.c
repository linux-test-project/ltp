/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_condattr_destroy()
 *   If it fails, an error number shall be returned to indicate the error:
 *   [EINVAL]  The value specified by 'attr' is invalid
 *
 * Steps:
 *     Try to destroy a NULL condition variable attributes object using pthread_condattr_destroy().
 *     If it returns EINVAL, the test passes.
 *
 */

/* This test explicitly tries to pass null to a parameter, that should not be NULL */
#pragma GCC diagnostic ignored "-Wnonnull"

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_condattr_t *condattr = NULL;
	int rc;

	/* Try to destroy a NULL condition variable attributes object using pthread_condattr_destroy()
	 * It should return EINVAL */
	if ((rc = pthread_condattr_destroy(condattr)) == EINVAL) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf
		    ("Test PASSED: *NOTE: Expect %d(EINVAL), but return %d, though standard states 'may' fail\n",
		     EINVAL, rc);
		return PTS_PASS;
	}
}
