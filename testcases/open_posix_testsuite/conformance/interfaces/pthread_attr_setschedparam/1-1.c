/*   
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * pthread_attr_setschedparam()

 * 1. Create a pthread_attr object and initialize it
 * 2. Set the policy and priority in that object
 * 3. Create a thread with this object
 * Test SCHED_FIFO 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_setschedparam"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define FIFOPOLICY SCHED_FIFO 

volatile int thread_created = 0;

void *thread_func()
{
	thread_created = 1;
	pthread_exit(0);
	return (void*)(0);
}

int main()
{
	pthread_t              thread;
	pthread_attr_t         attr;
 	void                   *status;
 	int                    rc=0;
	int                    policy = FIFOPOLICY;
	struct sched_param     param;
	int                    priority;

	rc = pthread_attr_init(&attr);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}
		
	rc = pthread_attr_setschedpolicy(&attr, policy);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_FAIL);
	}
	
	priority = sched_get_priority_max(policy);
	if(priority == -1) {
		printf(ERROR_PREFIX "sched_priority_get_max\n");	
 		exit(PTS_FAIL);
	}
	param.sched_priority = priority;
	rc = pthread_attr_setschedparam(&attr, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
 		exit(PTS_FAIL);
	}
	
	rc = pthread_create(&thread, &attr, thread_func, NULL);
	if(rc != 0) {
		if (rc == EPERM) {
			printf(ERROR_PREFIX "Permission Denied when creating thread with policy %d\n", policy);
			exit(PTS_UNRESOLVED);
		} else {
			printf(ERROR_PREFIX "pthread_create()\n");
			exit(PTS_FAIL);
		}
	}

	pthread_join(thread, &status);
	pthread_attr_destroy(&attr);

	if(thread_created == 0 ) {
		printf(ERROR_PREFIX "Thread was not created\n");
		exit(PTS_FAIL);
	}
	
	printf("Test PASS\n");
	exit(PTS_PASS);	
}
