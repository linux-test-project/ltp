/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_getspecific()
 *
 * It shall return the thread-specific data value associated with the given 'key'.  If no
 * thread-specific data value is associated with 'key, then the value NULL shall be returned.
 * It does not return any errors.
 *
 * Steps:
 * 1.  Create pthread_key_t object and do no specify a key accociated with this key
 * 2.  Call pthread_getspecific() and check that the value returns NULL.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	pthread_key_t key;
	void* rc;

	if(pthread_key_create(&key, NULL) != 0)
	{
		printf("Error: pthread_key_create() failed\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_getspecific(key);
	if(rc != NULL)
	{
		printf("Test FAILED: Did not return correct value, expected NULL, but got %ld\n", (long)rc);
		return PTS_FAIL;
	}
		
	if(pthread_key_delete(key) != 0)
	{
		printf("Error: pthread_key_delete() failed\n");
		return PTS_UNRESOLVED;
	}
		
	printf("Test PASSED\n");
	return PTS_PASS;
}
