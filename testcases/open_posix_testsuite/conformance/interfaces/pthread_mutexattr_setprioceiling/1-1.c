/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setprioceiling()
 *
 * Sets the priority ceiling attribute of a mutexattr object (which was prev. created
 * by the function pthread_mutexattr_init()).
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Get the min and max boundries for SCHED_FIFO of what prioceiling can be.
 * 3.  In a for loop, go through each valid SCHED_FIFO value, set the prioceiling.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include "posixtest.h"

int main()
{
	
	/* Make sure there is prioceiling capability. */ 
	/* #ifndef _POSIX_PRIORITY_SCHEDULING
	  fprintf(stderr,"prioceiling attribute is not available for testing\n");
	  return PTS_UNRESOLVED;	
	#endif */

	pthread_mutexattr_t mta;
	int max_prio, min_prio, i;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Get the max and min prio according to SCHED_FIFO (posix scheduling policy) */
	max_prio = sched_get_priority_max(SCHED_FIFO);
	min_prio = sched_get_priority_min(SCHED_FIFO);

	for(i=min_prio;(i<max_prio+1);i++)
	{
		/* Set the prioceiling to a priority number in the boundries 
		 * of SCHED_FIFO policy */
		if(pthread_mutexattr_setprioceiling(&mta,i))
		{
			printf("Test FAILED: Error setting prioceiling to %d\n", i);
			return PTS_FAIL;
		}

	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
