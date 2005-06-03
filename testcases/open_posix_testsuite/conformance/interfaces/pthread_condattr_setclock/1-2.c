/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_setclock()
 *  shall set the 'clock' attribute in an initialized attributes object referenced by 
 *  'attr'. 
 *
 * Steps:
 * 1.  Initialize a pthread_condattr_t object
 * 2.  Set the clock to CLOCK_MONOTONIC (if the kernel has that capability)
 * 3.  Check that it was set successfully
 *
 * *FIX-ME: Need to find a better check on whether the kernel supports CLOCK_MONOTONIC
 *  clocks or not.
 * 
 */

# define _XOPEN_SOURCE  600

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main()
{
	pthread_condattr_t condattr;
	struct timespec ts;
	int rc;

	/* Check whether kernel supports CLOCK_MONOTONIC */
	rc = clock_getres(CLOCK_MONOTONIC, &ts);
	if(rc != 0)
	{
		printf("UNSUPPORTED: CLOCK_MONOTONIC is unsupported\n");
		return PTS_UNSUPPORTED;
	}

	/* Initialize a cond attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0)
	{
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
	rc = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
	if(rc != 0)
	{
		printf("Test FAILED: Could not set clock to CLOCK_MONOTONIC\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
