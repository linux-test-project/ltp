/* 
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that pthread_condattr_destroy()
 *    shall destroy a condition variable attributes object.
 * 
 * Steps:
 * 1.  Initialize a pthread_condattr_t object using pthread_condattr_init()
 * 2.  Destroy the attributes object using pthread_condattr_destroy()
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main()
{
	pthread_condattr_t condattr;
	int rc;

	/* Initialize a condition variable attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0)
	{
		fprintf(stderr,"Cannot initialize condition variable attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the condition variable attributes object */
	if(pthread_condattr_destroy(&condattr) != 0)
	{
		fprintf(stderr,"Error at pthread_condattr_destroy(), rc=%d\n", rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;	
}


