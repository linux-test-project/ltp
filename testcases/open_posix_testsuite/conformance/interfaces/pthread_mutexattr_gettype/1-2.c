/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_gettype()
 *
 * Gets the mutex 'type' attribute.  This attribute is set in the 'type' parameter to 
 * these functions.  The default value is PTHREAD_MUTEX_DEFAULT.
 *
 * Testing the PTHREAD_MUTEX_DEFAULT type.
 * 
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Set tye mutexattr type to PTHREAD_MUTEX_DEFAULT
 * 3.  Call pthread_mutexattr_gettype() to check if type
 *     attribute is set as the default value PTHREAD_MUTEX_DEFAULT.
 * 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
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

	/* Set the mutex attribute 'type' to PTHREAD_MUTEX_DEFAULT. */
	if(pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_DEFAULT) != 0)
	{
		fprintf(stderr,"pthread_mutexattr_settype(): Error setting the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	 /* The 'type' attribute should be PTHREAD_MUTEX_DEFAULT  */
	if(pthread_mutexattr_gettype(&mta, &type) != 0)
	{
		fprintf(stderr,"pthread_mutexattr_gettype(): Error obtaining the attribute 'type'\n");
		return PTS_UNRESOLVED;
	}
	
	if(type != PTHREAD_MUTEX_DEFAULT)
	{
		printf("Test FAILED: Incorrect mutexattr 'type' value: %d. Should be PTHREAD_MUTEX_DEFAULT\n", type);
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;
}
