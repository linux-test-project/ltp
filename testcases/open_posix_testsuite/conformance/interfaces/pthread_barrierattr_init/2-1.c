/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrierattr_init()
 *
 * After a barrier attributes object has been used to initialize one or more barriers
 * any function affecting the attributes object (including destruction) shall not 
 * affect any previously initialized barrier.
 *
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define BARRIER_NUM 100

int main()
{
	int rc;
	pthread_barrierattr_t ba;
	pthread_barrier_t barriers [BARRIER_NUM];
	int cnt;
	
	/* Initialize the barrier attribute object */
	rc = pthread_barrierattr_init(&ba);
	if(rc != 0)
	{
		printf("Test FAILED: Error while initialize attribute object\n");
		return PTS_FAIL;
	}

	/* Initialize BARRIER_NUM barrier objects, with count==1 */
	for(cnt = 0; cnt < BARRIER_NUM; cnt++)
	{
		if(pthread_barrier_init(&barriers[cnt], &ba, 1) != 0)
		{
			printf("Error at %dth initialization\n", cnt);
			return PTS_UNRESOLVED;	
		}
	}

	/* Destroy barrier attribute object */
	rc = pthread_barrierattr_destroy(&ba);
	if(rc != 0)
	{
		printf("Error at pthread_barrierattr_destroy() "
			"return code: %d, %s", rc, strerror(rc));
		return PTS_UNRESOLVED;
	}

	/* Check that pthread_barrier_wait can still be performed, even after the attributes
	 * object has been destroyed */
	for(cnt = 0; cnt < BARRIER_NUM; cnt++)
	{
		rc = pthread_barrier_wait(&barriers[cnt]);
		if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
		{
			printf("Test Fail: Error at %dth wait, %s\n", cnt, strerror(rc));
			return PTS_FAIL;	
		}
	}

	/* Cleanup */
	for(cnt = 0; cnt < BARRIER_NUM; cnt++)
	{
		rc = pthread_barrier_destroy(&barriers[cnt]);
		if(rc != 0)
		{
			printf("Error at %dth destruction, %s\n", cnt, strerror(rc));
			return PTS_UNRESOLVED;	
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
