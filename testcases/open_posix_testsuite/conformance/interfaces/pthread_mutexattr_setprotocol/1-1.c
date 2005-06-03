/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setprotocol()
 *
 * Sets the protocol attribute of a mutexattr object (which was prev. created
 * by the function pthread_mutexattr_init()).
 * 
 * Steps:
 * 1. In a for loop, call pthread_mutexattr_setprotocol with all the valid 'protocol' values.
 * 2. In the for loop, then call pthread_mutexattr_getprotocol and ensure that the same
 *    value that was set was the same value that was retrieved from this function.
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include "posixtest.h"

int main()
{
	
	pthread_mutexattr_t mta;
	int protocol, protcls[3],i;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	protcls[0]=PTHREAD_PRIO_NONE;
	protcls[1]=PTHREAD_PRIO_INHERIT;
	protcls[2]=PTHREAD_PRIO_PROTECT;
	
	for(i=0;i<3;i++)
	{
		/* Set the protocol to one of the 3 valid protocols. */
		if(pthread_mutexattr_setprotocol(&mta,protcls[i]) != 0)
		{
			printf("Error setting protocol to %d\n", protcls[i]);
			return PTS_UNRESOLVED;
		}

		/* Get the protocol mutex attr. */
		if(pthread_mutexattr_getprotocol(&mta, &protocol) != 0)
		{
			fprintf(stderr,"Error obtaining the protocol attribute.\n");
			return PTS_UNRESOLVED;
		}

		/* Make sure that the protocol set is the protocl we get when calling
		 * pthread_mutexattr_getprocol() */
		if(protocol != protcls[i])
		{
			printf("Test FAILED: Set protocol %d, but instead got protocol %d.\n", protcls[i], protocol);
			return PTS_FAIL;
		}	
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
