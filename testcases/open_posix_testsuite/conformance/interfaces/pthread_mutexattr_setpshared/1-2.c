/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setpshared()
 *
 * The default value of the 'pshared' attribute is PTHREAD_PROCESS_PRIVATE.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutexattr_getpshared() to check if the process-shared 
 *     attribute is set as the default value PTHREAD_PROCESS_PRIVATE.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	  fprintf(stderr,"process-shared attribute is not available for testing\n");
	  return PTS_UNRESOLVED;	
	#endif

	pthread_mutexattr_t mta;
	int pshared;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* The default 'pshared' attribute should be PTHREAD_PROCESS_PRIVATE  */
	if(pthread_mutexattr_getpshared(&mta, &pshared) != 0)
	{
		fprintf(stderr,"Error obtaining the attribute process-shared\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure that the default is PTHREAD_PROCESS_PRIVATE. */	
	if(pshared != PTHREAD_PROCESS_PRIVATE)
	{
		printf("Test FAILED: Incorrect default pshared value: %d\n", pshared);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
