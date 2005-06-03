/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_condattr_destroy()
 *   A destroyed 'attr' attributes object can be reinitialized using
 *   pthread_condattr_init(); the results of otherwise referencing the 
 *   object after it has been destroyed are undefined.
 * 
 * Steps:
 * 1.  Initialize a pthread_condattr_t object using pthread_condattr_init()
 * 2.  Destroy that initialized attribute using pthread_condattr_destroy()
 * 3.  Initialize the pthread_condattr_t object again. This should not result
 *     in any error.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main()
{
	pthread_condattr_t condattr;

	/* Initialize a condition variable attributes object */
	if(pthread_condattr_init(&condattr) != 0)
	{
		fprintf(stderr,"Cannot initialize condition variable attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the condition variable attributes object */
	if(pthread_condattr_destroy(&condattr) != 0)
	{
		fprintf(stderr,"Cannot destroy the condition variable attributes object\n");
		return PTS_UNRESOLVED;
	}

	/* Initialize the condition variable attributes object again.  This shouldn't result in an error. */
	if(pthread_condattr_init(&condattr) != 0)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	else
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}


