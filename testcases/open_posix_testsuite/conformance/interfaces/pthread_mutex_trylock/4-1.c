/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutex_trylock()
 *   Upon failure, it shall return:
 *   -[EBUSY]   The mutex could not be acquired because it was already locked.

 * Steps: 
 *   -- Initilize a mutex object
 *   -- Lock the mutex using pthread_mutex_lock()
 *   -- Try to lock the mutex using pthread_mutex_trylock() and expect EBUSY
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

pthread_mutex_t    mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
  	int           	rc;

	if((rc=pthread_mutex_lock(&mutex))!=0) {
		fprintf(stderr,"Error at pthread_mutex_lock(), rc=%d\n",rc);
		return PTS_UNRESOLVED;
	}
	    		
   	rc = pthread_mutex_trylock(&mutex);
      	if(rc!=EBUSY) {
        	fprintf(stderr,"Expected %d(EBUSY), got %d\n",EBUSY,rc);
        	printf("Test FAILED\n");
		return PTS_FAIL;
      	}
    	
    	pthread_mutex_unlock(&mutex);
  	pthread_mutex_destroy(&mutex);

	printf("Test PASSED\n");
	return PTS_PASS;
}
