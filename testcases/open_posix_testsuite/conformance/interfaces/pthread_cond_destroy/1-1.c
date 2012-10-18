/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_cond_destroy()
 *   shall destroy the condition variable referenced by 'cond';
 *   the condition variable object in effect becomes uninitialized.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

pthread_cond_t  cond1, cond2;
pthread_cond_t  cond3 = PTHREAD_COND_INITIALIZER;

int main()
{
	pthread_condattr_t condattr;
	int rc;

	/* Initialize a condition variable attribute object */
	if ((rc=pthread_condattr_init(&condattr)) != 0) {
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize cond1 with the default condition variable attribute */
	if ((rc=pthread_cond_init(&cond1,&condattr)) != 0) {
		fprintf(stderr,"Fail to initialize cond1, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize cond2 with NULL attributes */
	if ((rc=pthread_cond_init(&cond2,NULL)) != 0) {
		fprintf(stderr,"Fail to initialize cond2, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Destroy the condition variable attribute object */
	if ((rc=pthread_condattr_destroy(&condattr)) != 0) {
		fprintf(stderr,"Error at pthread_condattr_destroy(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Destroy cond1 */
	if ((rc=pthread_cond_destroy(&cond1)) != 0) {
		fprintf(stderr,"Fail to destroy cond1, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Destroy cond2 */
	if ((rc=pthread_cond_destroy(&cond2)) != 0) {
		fprintf(stderr,"Fail to destroy cond2, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Destroy cond3 */
	if ((rc=pthread_cond_destroy(&cond3)) != 0) {
		fprintf(stderr,"Fail to destroy cond3, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
