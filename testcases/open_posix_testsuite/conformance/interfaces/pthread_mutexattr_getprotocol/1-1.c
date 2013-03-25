/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_getprotocol()
 *
 * Gets the protocol attribute of a mutexattr object (which was prev. created
 * by the function pthread_mutexattr_init()).
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutexattr_getprotocol() to obtain the protocol.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include "posixtest.h"

int main(void)
{

	pthread_mutexattr_t mta;
	int protocol, rc;

	/* Initialize a mutex attributes object */
	if (pthread_mutexattr_init(&mta) != 0) {
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Get the protocol mutex attr. */
	if ((rc = pthread_mutexattr_getprotocol(&mta, &protocol)) != 0) {
		printf
		    ("Test FAILED: Error in pthread_mutexattr_getprotocol rc=%d\n",
		     rc);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
