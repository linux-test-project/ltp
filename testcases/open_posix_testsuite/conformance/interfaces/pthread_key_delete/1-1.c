/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_key_delete()
 *
 * Shall delete a thread-specific data key previously returned by pthread_key_create.  The
 * thread-specific data values specified data values associated with 'key' need not be NULL at
 * the time pthread_key_delete is called.  It is the responsibility of the application to free
 * any application storage or perform any cleanup actions for data structures related to the 
 * deleted key or associated thread-specific data in any threads; this cleanup can be done
 * either before or after pthread_key_delete is called.  Any attempt to use 'key' following
 * the call to pthread_key_delete results in undefined behavior. 
 *
 * Steps:
 * 1. Create many keys, and do not specify and value to them (default is NULL)
 * 2. Delete the keys with pthread_key_delete
 * 3. Verify that this will not result in an error
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define NUM_OF_KEYS 10

int main()
{
	pthread_key_t keys[NUM_OF_KEYS];
	int i;

	for(i = 0;i<NUM_OF_KEYS;i++)
	{
		if(pthread_key_create(&keys[i], NULL) != 0)
		{
			printf("Error: pthread_key_create() failed\n");
			return PTS_UNRESOLVED;
		}

		if(pthread_key_delete(keys[i]) != 0)
		{
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}


	printf("Test PASSED\n");
	return PTS_PASS;
}
