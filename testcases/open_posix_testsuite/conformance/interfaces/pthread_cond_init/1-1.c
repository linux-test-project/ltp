/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_init()
 *   shall initialize the condition variable referenced by cond with attributes
 *   referenced by attr. If attr is NULL, the default condition variable attributes 
 *   shall be used; the effect is the same as passing the address of a default 
 *   condition variable attributes object. 
   
 * NOTE: There is no direct way to judge if two condition variables are equal,
 *       so this test does not cover the statement in the last sentence.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_condattr_t condattr;
	pthread_cond_t  cond1, cond2;
	int rc;

	/* Initialize a condition variable attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0) {
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	
	/* Initialize cond1 with the default condition variable attributes */
	if((rc=pthread_cond_init(&cond1,&condattr)) != 0) {
		fprintf(stderr,"Fail to initialize cond1, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Initialize cond2 with NULL attributes */
	if((rc=pthread_cond_init(&cond2,NULL)) != 0) {
		fprintf(stderr,"Fail to initialize cond2, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
