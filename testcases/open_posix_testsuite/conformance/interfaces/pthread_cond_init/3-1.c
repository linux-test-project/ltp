/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_init()
 *   Upon succesful completion, it shall return a 0
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_condattr_t condattr;
	pthread_cond_t  cond;
	int rc;

	/* Initialize a cond attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0) {
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	
	/* Initialize a cond object with the default cond attributes */
	if((rc=pthread_cond_init(&cond,&condattr)) == 0) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	
	/* Check if returned values are tolerable */
	else if(rc == ENOMEM) {
		fprintf(stderr,"Insufficient memory to initialize the cond\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EAGAIN) {
		fprintf(stderr,"Lack of the necessary resources to initilize the cond\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EBUSY) {
		fprintf(stderr,"Detected an attemp to reinitilize a previously initilized cond\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else 
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
