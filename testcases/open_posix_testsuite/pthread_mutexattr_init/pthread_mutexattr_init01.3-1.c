/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_init()
 *   Upon successful completion, pthread_mutexattr_init() shall return a value of 0.

 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  ENOMEM is the only error it returns, so if it doesn't return that error,
 *     the return number should be 0. 
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
	if((rc=pthread_mutexattr_init(&mta)) == 0)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	/* Insufficient memory exists to initialize the mutex attributes object */
	else if(rc == ENOMEM)
	{
		fprintf(stderr,"pthread_mutexattr_init() returns ENOMEM\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else 
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
