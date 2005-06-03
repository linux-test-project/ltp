/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_join()
 *  
 * pthread_join() SHALL fail if:
 *
 * -[ESRCH] No thread could be found corresponding to that thread ID  
 *
 * pthread_join() MAY fail if:

 * -[EINVAL] The implementation has detected that the value specified by
 *  'thread' does not refer to a joinable thread.
 * -[EDEADLK] A deadlock was detected or the value of 'thread' specifies the 
 *  calling thread.
  
 * It shall not return an error code of [EINTR]
 *
 * Testing ESRCH
 *
 * Steps:
 * 1.  Create a new thread.
 * 2.  Call pthread_join() in main. This means that the thread should have ended execution.
 * 3.  Call pthread_join() again, it should give the error code ESRCH.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

/* Thread's function. */
void *a_thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	int ret;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to return */
	ret=pthread_join(new_th, NULL);
	
	if(ret != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Now that the thread has returned, try to join it again.  It should give the error
	 * code of ESRCH. */
	ret=pthread_join(new_th, NULL);
	
	if(ret != ESRCH)
	{
		printf("Test FAILED: Return code should be ESRCH, but is: %d instead.\n", ret);
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}


