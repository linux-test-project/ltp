/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutexattr_settype()
 *
 * It shall fail if:
 * [EINVAL] - The value 'type' is invalid.

 * It may fail if:
 * [EINVAL] - The value specified by 'attr' is invalid.

 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object.
 * 2.  Pass to pthread_mutexattr_settype an invalid type.  It shall fail.
 * 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_mutexattr_t mta;
	int ret;
        int invalid_type = -1;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	

        while(invalid_type == PTHREAD_MUTEX_NORMAL || 
		invalid_type == PTHREAD_MUTEX_ERRORCHECK || 
		invalid_type == PTHREAD_MUTEX_RECURSIVE || 
		invalid_type == PTHREAD_MUTEX_DEFAULT)
        {
                invalid_type --;
        }
	 /* Set the 'type' attribute to be a negative number.  */
	ret=pthread_mutexattr_settype(&mta, invalid_type);
	
	if (ret != EINVAL)
	{
		printf("Test FAILED: Expected return code of EINVAL, got: %d\n", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
