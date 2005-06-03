/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_getprioceiling()
 *
 * returns the current prioceiling of the mutex.
 *
 * Steps:i
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutex_getprioceiling() to obtain the prioceiling.
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

	pthread_mutex_t mutex;
	int prioceiling, max_prio, min_prio;
	
	/* Initialize a mutex object */
	if(pthread_mutex_init(&mutex, NULL) != 0)
	{
		perror("Error at pthread_mutex_init()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Get the prioceiling of the mutex. */
	if(pthread_mutex_getprioceiling(&mutex, &prioceiling) != 0)
	{
		printf("Test FAILED: Error obtaining the priority ceiling\n");
		return PTS_FAIL;
	}
	
	/* Get the max and min prio according to SCHED_FIFO (posix scheduling policy) */
	max_prio = sched_get_priority_max(SCHED_FIFO);
	min_prio = sched_get_priority_min(SCHED_FIFO);

	/* Make sure that prioceiling is withing the legal SCHED_FIFO boundries. */
	if((prioceiling < min_prio) || (prioceiling > max_prio))
	{
		printf("Test FAILED: Default prioceiling %d is not compliant with SCHED_FIFO boundry. \n", prioceiling);
		return PTS_FAIL;
	}

	printf("Test PASSED: Prioceiling %d\n", prioceiling);
	return PTS_PASS;
}
