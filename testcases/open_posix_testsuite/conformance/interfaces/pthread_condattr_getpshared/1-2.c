/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_getpshared()
 *
 *  It shall obtain the value of the process-shared attribute from 'attr'.
 *
 * Steps:
 * 1.  In a loop, initialize a pthread_condattr_t object with pthread_condattr_init()
 * 2.  Set 'pshared' of the object to PTHREAD_PROCESS_SHARED.
 * 3.  Call pthread_condattr_getpshared() to check if the process-shared 
 *     attribute is set as PTHREAD_PROCESS_SHARED.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define NUM_OF_CONDATTR 10

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	  fprintf(stderr,"process-shared attribute is not available for testing\n");
	  return PTS_UNRESOLVED;	
	#endif

	pthread_condattr_t attr[NUM_OF_CONDATTR];
	int ret, i, pshared;
	
	for(i=0;i<NUM_OF_CONDATTR;i++)
	{
		/* Initialize a cond attributes object */
		if(pthread_condattr_init(&attr[i]) != 0)
		{
			perror("Error at pthread_condattr_init()\n");
			return PTS_UNRESOLVED;
		}

		/* Set 'pshared' to PTHREAD_PROCESS_SHARED. */
		ret=pthread_condattr_setpshared(&attr[i], PTHREAD_PROCESS_SHARED);
		if(ret != 0)
		{
			printf("Error in pthread_condattr_setpshared(), error: %d\n", ret);
			return PTS_UNRESOLVED;
		}
	
		/* Get 'pshared'.  It should be PTHREAD_PROCESS_SHARED. */
		if(pthread_condattr_getpshared(&attr[i], &pshared) != 0)
		{
			fprintf(stderr,"Error obtaining the attribute process-shared\n");
			return PTS_UNRESOLVED;
		}
	
		if(pshared != PTHREAD_PROCESS_SHARED)
		{
			printf("Test FAILED: Incorrect pshared value: %d\n", pshared);
			return PTS_FAIL;
		}
	
		/* Destory the cond attributes object */
		if(pthread_condattr_destroy(&attr[i]) != 0)
		{
			perror("Error at pthread_condattr_destroy()\n");
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
