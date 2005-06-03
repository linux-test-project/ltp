/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * This test case initializes an attr and sets its schedpolicy, 
 * then create a thread with the attr.
 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

void *thread_func()
{
	pthread_exit(0);
	return (void*)(0);
}

int main()
{
	pthread_t thread;
	pthread_attr_t attr;
    	void *status;
    	int rc;
	int policy = SCHED_FIFO;

	if(pthread_attr_init(&attr) != 0) {
		printf("Error on pthread_attr_init()\n");
		return PTS_UNRESOLVED;
	}
		
	if((rc=pthread_attr_setschedpolicy(&attr,policy)) != 0) {
    		printf("Error on pthread_attr_setschedpolicy()\t rc=%d\n", rc);
    		return PTS_FAIL;
    	}
		
    	if((rc=pthread_create(&thread,&attr,thread_func,NULL)) != 0) {
  		if (rc == EPERM) {
  	    		printf("Permission Denied when creating thread with policy %d\n", policy);
  	    		return PTS_UNRESOLVED;
  	    	} else {
	    		printf("Error on pthread_create()\t rc=%d\n", rc);
    			return PTS_FAIL;
    		}
    	}

	pthread_join(thread, &status);
	pthread_attr_destroy(&attr);
	return PTS_PASS;	
}
