/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_getstackaddr()
 * 
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr) 
 * 2.  set the stackaddr to attr
 * 3.  get the stackaddr 
 */

#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_getstackaddr"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main()
{
	pthread_attr_t attr;
	size_t ssize;
	void *stack_addr;
	void *saddr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}
	
	/* Get the default stack_addr value */	
	rc = pthread_attr_getstackaddr(&attr, &stack_addr); 	
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_getstackaddr");
		exit(PTS_UNRESOLVED);
	}
	printf("stack_addr = %u\n", stack_addr);

	ssize = PTHREAD_STACK_MIN;

	if (posix_memalign (&stack_addr, sysconf(_SC_PAGE_SIZE), 
            ssize) != 0)
    	{
      		perror (ERROR_PREFIX "out of memory while "
                        "allocating the stack memory");
      		exit(PTS_UNRESOLVED);
    	}
	printf("stack_addr = %u\n", stack_addr);

	rc = pthread_attr_setstackaddr(&attr, stack_addr);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_setstacksize");
                exit(PTS_UNRESOLVED);
        }

	rc = pthread_attr_getstackaddr(&attr, &saddr);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_getstackaddr");
                exit(PTS_UNRESOLVED);
        }
	printf("saddr = %u\n", saddr);

	rc = pthread_attr_destroy(&attr);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
        }
	
	printf("Test PASSED\n");
	return PTS_PASS;
}


