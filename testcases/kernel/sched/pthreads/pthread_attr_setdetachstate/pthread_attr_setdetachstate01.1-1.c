/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_attr_setdetachstate()
 *  
 * shall set the detachstate attribute in the 'attr' object.  
 * The detach state is either PTHREAD_CREATE_DETACHED or 
 * PTHEAD_CREATE_JOINABLE.  
  
 * A value of PTHREAD_CREATE_DETACHED shall cause all threads created with 
 * 'attr'  to be in the detached state, wheras using a value of 
 * PTHREAD_CREATE_JOINABLE shall cause threads created with 'attr' to be in 
 * the joinable state.
 * The default value of the detachstate attribute shall be
 * PTHREAD_CREATE_JOINABLE. 
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Using pthread_attr_setdetachstate(), set the detachstate to 
 *     PTHREAD_CREATE_JOINABLE. 
 * 3.  Get the detachedstate and make sure that it is really set to 
 *     PTHREAD_CREATE_JOINABLE.      
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
	
	/* Set the attribute object to PTHREAD_CREATE_JOINABLE. */
	if(pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_JOINABLE) != 0)
	{
		perror("Error in pthread_attr_setdetachstate()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Check to see if the detachstate is truly PTHREAD_CREATE_JOINABLE. */
	if(pthread_attr_getdetachstate(&new_attr, &detach_state) != 0)
	{
		perror("Error in pthread_attr_getdetachstate.\n");
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


