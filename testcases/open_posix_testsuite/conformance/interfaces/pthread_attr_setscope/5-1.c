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

#define TEST "5-1"
#define FUNCTION "pthread_attr_setscope"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

/* What is the unsupported value of scope paramter? */
#define UNSUPSCOPE -1 

int main()
{
	if (1) {
		printf("Untested for now, cannot find a unsupported inheritsched value\n");	
		return PTS_UNTESTED;
	}
	int                   rc=0;
	pthread_attr_t        attr;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		exit(PTS_UNRESOLVED);
	}

  	rc = pthread_attr_setscope(&attr, UNSUPSCOPE);
	if ((rc != ENOTSUP)) {
		printf(ERROR_PREFIX "pthread_attr_setscope\n");
		exit(PTS_UNRESOLVED);
	}
  	rc = pthread_attr_destroy(&attr);
	if( rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_destroy\n");
		exit(PTS_UNRESOLVED);
	}
	printf("Test PASS\n");
	return PTS_PASS;
}




