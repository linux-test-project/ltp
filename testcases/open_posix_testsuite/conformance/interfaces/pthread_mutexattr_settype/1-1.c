/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutexattr_settype()
 * int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
 * Sets the mutex 'type' attribute.  This attribute is set in the 'type' parameter to 
 * these functions.  The default value is PTHREAD_MUTEX_DEFAULT. 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_mutexattr_t mta;
	int type;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	if(pthread_mutexattr_gettype(&mta, &type) != 0)
	{
		printf("Error getting the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	if(type != PTHREAD_MUTEX_DEFAULT)
	{
		printf("Test FAILED: Default value of the 'type' attribute is not PTHREAD_MUTEX_DEFAULT \n");
		return PTS_FAIL;		
	}	
	
	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL) != 0)
	{
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}
	
	if(pthread_mutexattr_gettype(&mta, &type) != 0)
	{
		printf("Error getting the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	if(type != PTHREAD_MUTEX_NORMAL)
	{
		printf("Test FAILED: Type not correct get/set \n");
		return PTS_FAIL;		
	}	

	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK) != 0)
	{
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}
	if(pthread_mutexattr_gettype(&mta, &type) != 0)
	{
		printf("Error getting the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	if(type != PTHREAD_MUTEX_ERRORCHECK)
	{
		printf("Test FAILED: Type not correct get/set \n");
		return PTS_FAIL;		
	}	

	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE) != 0)
	{
		printf("Test FAILED: Error setting the attribute 'type'\n");
		return PTS_FAIL;
	}
	
	if(pthread_mutexattr_gettype(&mta, &type) != 0)
	{
		printf("Error getting the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	if(type != PTHREAD_MUTEX_RECURSIVE)
	{
		printf("Test FAILED: Type not correct get/set \n");
		return PTS_FAIL;		
	}	

	if(pthread_mutexattr_destroy(&mta) != 0)
	{
		printf("Error at pthread_mutexattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

