/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_getscope()
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setscope with contentionscope parameter
 * 3.  Call pthread_attr_getscope to get the contentionscope

 * NOTE: The contension scope value is a may requirement.  
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_getscope"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define SYSTEMSCOPE PTHREAD_SCOPE_SYSTEM
#define PROCESSSCOPE PTHREAD_SCOPE_PROCESS

int verify_scope(pthread_attr_t *attr, int scopetype) {
	int rc;
	int scope;

	rc = pthread_attr_getscope(attr, &scope);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_getscope");
		exit(PTS_UNRESOLVED);
	}
	switch(scopetype) {
	case SYSTEMSCOPE:
  		if (scope != SYSTEMSCOPE) {
    			perror(ERROR_PREFIX "got wrong scope param");
    			exit(PTS_FAIL);
  		}	
		break;
	case PROCESSSCOPE:
  		if (scope != PROCESSSCOPE) {
    			perror(ERROR_PREFIX "got wrong scope param");
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
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

  	rc = pthread_attr_setscope(&attr, SYSTEMSCOPE);
	if( rc != 0) {
		perror(ERROR_PREFIX "PTHREAD_SCOPE_SYSTEM is not supported");
	} else {
  		verify_scope(&attr, SYSTEMSCOPE);
	}
  	rc = pthread_attr_setscope(&attr, PROCESSSCOPE);
	if( rc != 0) {
		perror(ERROR_PREFIX "PTHREAD_SCOPE_SYSTEM is not supported");
	} else {
  		verify_scope(&attr, PROCESSSCOPE);
	}

  	rc = pthread_attr_destroy(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASS\n");
	return PTS_PASS;
}




