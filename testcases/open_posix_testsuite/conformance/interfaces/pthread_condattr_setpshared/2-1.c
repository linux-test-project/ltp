/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_setpshared()
 *
 * If successful, the pthread_condattr_setpshared() function shall return zero;
 * The pthread_condattr_setpshared() function may fail if:
 *   [EINVAL] The new value specified for the attribute is outside the range of legal values
 *            for that attribute.  
 *
 *
 * Steps:
 * 
 * 1. Pass to pthread_condattr_setpshared() a negative value for 'pshared'
 * 2. It may return EINVAL
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_PSHARED_VALUE -100

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	  fprintf(stderr,"process-shared attribute is not available for testing\n");
	  return PTS_UNRESOLVED;	
	#endif

	pthread_condattr_t attr;
	int ret;
	
	/* Initialize a cond attributes object */
	if(pthread_condattr_init(&attr) != 0)
	{
		perror("Error at pthread_condattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Set 'pshared' to INVALID_PSHARED_VALUE. */
	ret=pthread_condattr_setpshared(&attr, INVALID_PSHARED_VALUE);
	if(ret != 0)
	{
		if(ret == EINVAL)
		{
			printf("Test PASSED\n");
			return PTS_PASS;
		}
		
		printf("Test FAILED: Invalid return code, expected 0 or EINVAL, but got: %d\n", ret);
		return PTS_FAIL;
	}
	
	/* Destory the cond attributes object */
	if(pthread_condattr_destroy(&attr) != 0)
	{
		perror("Error at pthread_condattr_destroy()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED: NOTE*: Returned 0 when passed an invalid 'pshared', but standard says 'may' fail.\n");
	return PTS_PASS;
}
