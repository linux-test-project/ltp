/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_getclock()
 *  shall get the 'clock' attribute of a condattr object. 
 *
 * Steps:
 * 1.  Initialize a pthread_condattr_t object
 * 2.  Set the clock attribute to CLOCK_REALTIME
 * 3.  Get the clock attribute
 * 4.  Check that it was successful
 * 
 */

# define _XOPEN_SOURCE  600

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_condattr_t condattr;
	clockid_t clockid;
	int rc;

	/* Initialize a cond attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0)
	{
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
	rc = pthread_condattr_setclock(&condattr, CLOCK_REALTIME);
	if(rc != 0)
	{
		perror("Error: Could not set clock to CLOCK_REALTIME\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_condattr_getclock(&condattr, &clockid);
	if(rc != 0)
	{
		printf("Test FAILED: Could not get the clock attribute\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
