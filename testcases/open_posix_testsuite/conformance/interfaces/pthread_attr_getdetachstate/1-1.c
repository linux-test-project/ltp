/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_attr_getdetachstate()
 * shall get the detachstate attribute in the 'attr' object.  The detach state
 * is either PTHREAD_CREATE_DETACHED or PTHEAD_CREATE_JOINABLE.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Using pthread_attr_getdetachstate(), get the detachstate of the
 *     attribute object.  It should be PTHREAD_CREATE_JOINABLE.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_attr_t new_attr;
	int detach_state;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0)
	{
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* The test passes if pthread_attr_getdetachstate gets the attribute
	 * of PTHREAD_CREATE_JOINABLE from the attribute object. */
	if (pthread_attr_getdetachstate(&new_attr, &detach_state) != 0)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (detach_state == PTHREAD_CREATE_JOINABLE)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	else
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
}
