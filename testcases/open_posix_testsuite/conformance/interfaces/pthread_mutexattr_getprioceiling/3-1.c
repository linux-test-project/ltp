/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_getprioceiling()
 *
 * It MAY fail if:
 *
 * [EINVAL] - 'attr' or 'prioceiling' is invalid.
 * [EPERM] - The caller doesn't have the privilege to perform the operation.
 *
 * This function shall not return an error code of [EINTR]
 *
 * Steps:
 * 1.  Call pthread_mutexattr_getprioceiling() to obtain the prioceiling for an 
 *     uninitialized pthread_mutexattr_t object.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	
	int prioceiling, ret;
	pthread_mutexattr_t mta;

	/* Get the prioceiling of an unintialized mutex attr. */
	if((ret=pthread_mutexattr_getprioceiling(&mta, &prioceiling)) == 0)
	{
		printf("Test PASSED: *Note: Returned 0 instead of EINVAL when passed an uninitialized mutex attribute object to pthread_mutexattr_getprioceiling, but standard says 'may' fail.\n");
		return PTS_PASS;		
	      	  
	}else if(ret != EINVAL)
	{
		printf("Test FAILED: Invalid return code %d. Expected EINVAL or 0.\n", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}


