/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_setclock()
 * It returns 0 upon success, and an error code if it fails:
 *
 * It may fail if:
 *
 * [EINVAL] - the 'attr' value is invalid
 * [EINVAL] - 'clock_id' doesn't refer to a known clock or is a CPU-time clock.
 *
 * Steps:
 * 1.  Initialize a pthread_condattr_t object
 * 2.  Set the clock to an invalid value
 * 3.  It should fail
 * 
 */

# define _XOPEN_SOURCE  600

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define INVALID_CLOCKID -100

int main()
{
	pthread_condattr_t condattr;
	int rc;

	/* Initialize a cond attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0)
	{
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
	rc = pthread_condattr_setclock(&condattr, INVALID_CLOCKID);
	if(rc != EINVAL)
	{
		printf("Test PASSED: *NOTE: Test passed while passing an invalid clockid, but the standard says 'may' fail\n");
		return PTS_PASS;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
