/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlockattr_getpshared()
 *
 *  It shall obtain the value of the process-shared attribute from 'attr'.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlockattr_t object with pthread_rwlockattr_init()
 * 2.  Call pthread_rwlockattr_getpshared() to check if the process-shared 
 *     attribute is set as the default value PTHREAD_PROCESS_PRIVATE.
 * 
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_rwlockattr_t rwla;
	int pshared;
	int rc = 0;
	
	#ifndef PTHREAD_PROCESS_SHARED
	  printf("process-shared attribute is not available for testing\n");
	  return PTS_UNSUPPORTED;	
	#endif
	
	/* Initialize a rwlock attributes object */
	if(pthread_rwlockattr_init(&rwla) != 0)
	{
		printf("Error at pthread_rwlockattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	 /* The default 'pshared' attribute should be PTHREAD_PROCESS_PRIVATE  */
	rc = pthread_rwlockattr_getpshared(&rwla, &pshared);
	if(rc != 0)
	{
		printf("Test FAILED: Error at pthread_rwlockattr_getpshared(): %d\n", rc);
		return PTS_FAIL;
	}
	
	if(pshared != PTHREAD_PROCESS_PRIVATE)
	{
		printf("Test FAILED: Incorrect default pshared value: %d\n", pshared);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
