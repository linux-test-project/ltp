/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_lock()
 *   Upon succesful completion, it shall return a 0
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_mutex_t  mutex;
	int rc;

	/* Initialize a mutex object with the default mutex attributes */
	if((rc=pthread_mutex_init(&mutex,NULL)) != 0) {
		fprintf(stderr,"Error at pthread_mutex_init(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	
	/* Lock the mutex using pthread_mutex_lock() */
	if((rc=pthread_mutex_lock(&mutex)) == 0) {
		pthread_mutex_unlock(&mutex);
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	
	/* Check if returned values are tolerable */
	else if(rc == EINVAL) {
		fprintf(stderr,"Invalid mutex object\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EAGAIN) {
		fprintf(stderr,"The maximum number of recursive locks has been exceeded\n");
		return PTS_UNRESOLVED;
	}
	else if(rc == EDEADLK) {
		fprintf(stderr,"The current thread already owns the mutex\n");
		return PTS_UNRESOLVED;
	}

	/* Any other returned value means the test failed */
	else 
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
