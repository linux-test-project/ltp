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
 * 1.  Initialize a pthread_condattr_t object with pthread_condattr_init()
 * 2.  Call pthread_condattr_getpshared() to check if the process-shared 
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

	pthread_condattr_t attr;
	int pshared;
	
	/* Initialize a mutex attributes object */
	if(pthread_condattr_init(&attr) != 0)
	{
		perror("Error at pthread_condattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* The default 'pshared' attribute should be PTHREAD_PROCESS_PRIVATE  */
	if(pthread_condattr_getpshared(&attr, &pshared) != 0)
	{
		fprintf(stderr,"Error obtaining the attribute process-shared\n");
		return PTS_UNRESOLVED;
	}
	
	if(pshared != PTHREAD_PROCESS_PRIVATE)
	{
		printf("Test FAILED: Incorrect default pshared value: %d\n", pshared);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
