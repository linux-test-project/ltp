/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrierattr_setpshared() 
 *
 * The pthread_barrierattr_setpshared( ) function may fail if:
 * [EINVAL] The new value specified for the process-shared attribute is not one of 
 * the legal values PTHREAD_PROCESS_SHARED or PTHREAD_PROCESS_PRIVATE.
 * This case will always pass
 */


#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	  fprintf(stderr,"process-shared attribute is not available for testing\n");
	  return PTS_UNSUPPORTED;	
	#endif

	pthread_barrierattr_t ba;
	int	pshared = PTHREAD_PROCESS_PRIVATE + 1;
	int	rc; 

	/* Set pshared to an invalid value */
	if(pshared == PTHREAD_PROCESS_SHARED)
	{
		pshared += 1;
	}

	/* Initialize a barrier attributes object */
	if(pthread_barrierattr_init(&ba) != 0)
	{
		printf("Error at pthread_barrierattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set process-shared attribute to an invalid value */
	rc = pthread_barrierattr_setpshared(&ba, pshared);
	if(rc == EINVAL)
		printf("Test PASSED\n");
	else
	{
		printf("Get return code: %d, %s\n", rc, strerror(rc));
		printf("Test PASSED: Note*: Expected EINVAL, but standard says 'may' fail.\n");
	}
	
	return PTS_PASS;
		
}
