/* 
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that pthread_mutexattr_destroy()
 *    shall destroy a mutex attributes object.
 * 
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object using pthread_mutexattr_init()
 * 2.  Destroy the attributes object using pthread_mutexattr_destroy()
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main()
{
	pthread_mutexattr_t mta;
	int rc;

	/* Initialize a mutex attributes object */
	if((rc=pthread_mutexattr_init(&mta)) != 0)
	{
		fprintf(stderr,"Cannot initialize mutex attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the mutex attributes object */
	if(pthread_mutexattr_destroy(&mta) != 0)
	{
		fprintf(stderr,"Error at pthread_mutexattr_destroy(), rc=%d\n", rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;	
}


