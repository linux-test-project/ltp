/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrier_init()
 *
 *
 * The pthread_barrier_init( ) function shall fail if:
 * [EINVAL] The value specified by count is equal to zero.
 *
 */

#define COUNT 0

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int main()
{
	int rc;
	pthread_barrier_t barrier;

	/* Intilized barrier with count 0 (it should return EINVAL) */

	rc = pthread_barrier_init(&barrier, NULL, COUNT);
	
	if(rc != EINVAL)
	{
		printf("Test FAILED: pthread_barrier_init() does not return EINVAL when intializing a barrier with count=0,"
			" return code %d, %s\n", rc, strerror(rc));
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
