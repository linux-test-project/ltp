/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_getschedpolicy()
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setschedpolicy with policy parameter
 * 3.  Call pthread_attr_getschedpolicy to get the policy
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_getschedpolicy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define FIFOPOLICY SCHED_FIFO 
#define RRPOLICY SCHED_RR
#define OTHERPOLICY SCHED_OTHER

int verify_policy(pthread_attr_t *attr, int policytype) {
	int rc;
	int policy;

	rc = pthread_attr_getschedpolicy(attr, &policy);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_getschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
	switch(policytype) {
	case SCHED_FIFO:
  		if (policy != FIFOPOLICY) {
    			printf(ERROR_PREFIX "got wrong policy param\n");
    			exit(PTS_FAIL);
  		}	
		break;
	case SCHED_RR:
  		if (policy != RRPOLICY) {
    			printf(ERROR_PREFIX "got wrong policy param\n");
    			exit(PTS_FAIL);
  		}	
		break;
	case SCHED_OTHER:
  		if (policy != OTHERPOLICY) {
    			printf(ERROR_PREFIX "got wrong policy param\n");
    			exit(PTS_FAIL);
  		}	
		break;
	}
        return 0;                                   
}

int main()
{
	int                   rc=0;
	pthread_attr_t        attr;

	rc = pthread_attr_init(&attr);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}

  	rc = pthread_attr_setschedpolicy(&attr, FIFOPOLICY);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
  	verify_policy(&attr, FIFOPOLICY);

  	rc = pthread_attr_setschedpolicy(&attr, RRPOLICY);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
  	verify_policy(&attr, RRPOLICY);

  	rc = pthread_attr_setschedpolicy(&attr, OTHERPOLICY);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		exit(PTS_UNRESOLVED);
	}
  	verify_policy(&attr, OTHERPOLICY);

  	rc = pthread_attr_destroy(&attr);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_destroy\n");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASS\n");
	return PTS_PASS;
}




