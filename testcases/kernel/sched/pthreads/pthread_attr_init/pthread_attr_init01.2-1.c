/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test the resulting attributes object (possibly modified by setting individual
 * attribute values) when used by pthread_create() defines the attributes of
 * the thread created.
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Pass the newly created attribute object to pthread_create()
 * 3.  Test that thread is joinable, since using pthread_attr_init() will
 *     set the default detachstate to PTHREAD_CREATE_JOINABLE.
 * 4.  Use pthread_join and pthread_detach() to test this.  They should both
 *     not return errors since the thread should be joinable.  If they do 
 *     return an error, that means that the thread is not joinable, but rather
 *     in a detached state, and the test fails.              
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


void *a_thread_func()
{
	
	pthread_exit(0);
}

int main()
{
	pthread_t new_th;
	pthread_attr_t new_attr;
	int ret_val;

	/* Initialize attribute */
	if(pthread_attr_init(&new_attr) != 0)
	{
		perror("Cannot initialize attribute object");
		return PTS_UNRESOLVED;
	}

	/* Create a new thread passing it the new attribute object */
	if(pthread_create(&new_th, &new_attr, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread");
		return PTS_UNRESOLVED;
	}

	/* If pthread_join() or pthread_detach fail, that means that the
	 * test fails as well. */
	ret_val=pthread_join(new_th, NULL);

	if(ret_val != 0)
	{
		/* Thread is detached and can't be joined */
		if(ret_val == EINVAL)
		{
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		/* pthread_join() failed for another reason */
		else
		{
			perror("Error in pthread_join()");
			return PTS_UNRESOLVED;
		}
	}

	ret_val=pthread_detach(new_th);

	if(ret_val != 0)
	{
		/* Thread is already detached. */
		if(ret_val == EINVAL)
		{
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		/* pthread_detach() failed for another reason. */
		else
		{
			perror("Error in pthread_detach()");
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
	
}


