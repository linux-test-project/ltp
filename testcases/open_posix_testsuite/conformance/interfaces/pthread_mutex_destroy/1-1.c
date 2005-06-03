/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_destroy()
 *   shall destroy the mutex referenced by 'mutex'; the mutex object in effect
 *   becomes uninitialized. 
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

pthread_mutex_t  mutex1, mutex2;
pthread_mutex_t  mutex3 = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	pthread_mutexattr_t mta;
	int rc;

	/* Initialize a mutex attributes object */
	if((rc=pthread_mutexattr_init(&mta)) != 0) {
		fprintf(stderr,"Error at pthread_mutexattr_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	
	/* Initialize mutex1 with the default mutex attributes */
	if((rc=pthread_mutex_init(&mutex1,&mta)) != 0) {
		fprintf(stderr,"Fail to initialize mutex1, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Initialize mutex2 with NULL attributes */
	if((rc=pthread_mutex_init(&mutex2,NULL)) != 0) {
		fprintf(stderr,"Fail to initialize mutex2, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Destroy the mutex attributes object */
	if((rc=pthread_mutexattr_destroy(&mta)) != 0) {
		fprintf(stderr,"Error at pthread_mutexattr_destroy(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Destroy mutex1 */
	if((rc=pthread_mutex_destroy(&mutex1)) != 0) {
		fprintf(stderr,"Fail to destroy mutex1, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Destroy mutex2 */
	if((rc=pthread_mutex_destroy(&mutex2)) != 0) {
		fprintf(stderr,"Fail to destroy mutex2, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Destroy mutex3 */
	if((rc=pthread_mutex_destroy(&mutex3)) != 0) {
		fprintf(stderr,"Fail to destroy mutex3, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
