/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_setscope()
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setscope with unsupported scope 
 *     parameter
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "pthread_attr_setscope"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define INVALIDSCOPE 999 

int main()
{
	int                   rc=0;
	pthread_attr_t        attr;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

  	rc = pthread_attr_setscope(&attr, INVALIDSCOPE);
	if ((rc != EINVAL)) {
		perror(ERROR_PREFIX "pthread_attr_setscope");
		exit(PTS_FAIL);
	}
  	rc = pthread_attr_destroy(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASS\n");
	return PTS_PASS;
}




