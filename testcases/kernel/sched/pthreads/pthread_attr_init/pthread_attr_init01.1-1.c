/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_attr_init()
 *  shall initialize a thread attributes object 'attr' with the default value
 *  for all the individual attributes used by a given implementation.

 *  NOTE: Since most default values are implementation specific, only the
 *  attribute of the detachstate will be tested since the default value
 *  is defined in the spec. (default value:  PTHREAD_CREATE_JOINABLE)
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Using pthread_attr_getdetachstate(), test that the default value
 *     of the detachstate is PTHREAD_CREATE_JOINABLE (the default).
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
	if(pthread_attr_init(&new_attr) != 0)
	{
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}
	
	/* The test passes if the attribute object has a detachstate of 
	 * PTHREAD_CREATE_JOINABLE, which is the default value for this
	 * attribute. */
	if(pthread_attr_getdetachstate(&new_attr, &detach_state) != 0)
	{
		perror("Error obtaining the detachstate of the attribute\n");
		return PTS_UNRESOLVED;
	}
	
	if(detach_state == PTHREAD_CREATE_JOINABLE)
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


