/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setpshared()
 *
 * It MAY fail if:
 *
 * [EINVAL] - 'attr' is invalid.
 *
 * Steps:
 *
 * 1. Pass to pthread_mutexattr_setpshared() an uninitialized attributes object.
 * 2. It may return the value of EINVAL.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	
	/* Make sure there is process-shared capability. */ 
	#ifndef PTHREAD_PROCESS_SHARED
	fprintf(stderr,"process-shared attribute is not available for testing\n");
	return PTS_UNRESOLVED;	
	#endif

	pthread_mutexattr_t mta;
	int ret;
	
	 /* Set the attribute to PTHREAD_PROCESS_PRIVATE.  */
	ret=pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_PRIVATE);
	if(ret != 0)
	{
		if(ret == EINVAL)
		{
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf("Test FAILED: Expected return code 0 or EINVAL, got: %d", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED: NOTE*: Returned 0 on error, though standard states 'may' fail.\n");
       	return PTS_PASS;
	
}
