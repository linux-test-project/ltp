/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrierattr_destroy()
 *
 * The pthread_barrierattr_destroy( ) function shall destroy a barrier 
 * attributes object. A destroyed attr attributes object can be reinitialized 
 *
 */


#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int main()
{
	int rc;
	pthread_barrierattr_t ba;
	
	/* Initialize a barrier attribute object */
	if(pthread_barrierattr_init(&ba) != 0)
	{
		printf("Error at pthread_barrierattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Destroy barrier attribute object */
	rc = pthread_barrierattr_destroy(&ba);
	if(rc != 0)
	{
		printf("Test FAILED: Error at pthread_barrierattr_destroy() "
			"return code: %d, %s", rc, strerror(rc));
		return PTS_FAIL;
	}

	/* Re-initialize the barrier attribute object */
	rc = pthread_barrierattr_init(&ba);
	if(rc != 0)
	{
		printf("Test FAILED: Error while re-initializing barrier attribute object\n");
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
