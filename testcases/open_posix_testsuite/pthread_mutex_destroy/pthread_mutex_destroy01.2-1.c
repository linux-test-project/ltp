/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutex_destroy() that
 *   a destroyed mutex object can be reinitialized using pthread_mutex_init()
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_mutex_t mutex;

	/* Initialize a mutex attributes object */
	if(pthread_mutex_init(&mutex,NULL) != 0)
	{
		fprintf(stderr,"Cannot initialize mutex object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy the mutex attributes object */
	if(pthread_mutex_destroy(&mutex) != 0)
	{
		fprintf(stderr,"Cannot destroy the mutex object\n");
		return PTS_UNRESOLVED;
	}

	/* Initialize the mutex attributes object again.  This shouldn't result in an error. */
	if(pthread_mutex_init(&mutex,NULL) != 0)
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
