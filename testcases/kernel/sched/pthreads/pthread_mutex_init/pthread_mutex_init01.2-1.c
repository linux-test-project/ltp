/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_init()
 *   Upon successful initialization, the state of the mutex becomes 
 *   initialized and unlocked.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	pthread_mutex_t  mutex;
	int rc;

	/* Initialize a mutex object */
	if((rc=pthread_mutex_init(&mutex,NULL)) != 0) {
		fprintf(stderr,"Fail to initialize mutex, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* Acquire the mutex object using pthread_mutex_lock */
	if((rc=pthread_mutex_lock(&mutex)) != 0) {
		fprintf(stderr,"Fail to lock the mutex, rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	fprintf(stderr,"Main: hold the mutex for a while\n");
	sleep(1);

	/* Release the mutex object using pthread_mutex_unlock */
	if((rc=pthread_mutex_unlock(&mutex)) != 0) {
		fprintf(stderr,"Fail to unlock the mutex, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	/* Destory the mutex object */
	if((rc=pthread_mutex_destroy(&mutex)) != 0) {
		fprintf(stderr,"Fail to destory the mutex, rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
