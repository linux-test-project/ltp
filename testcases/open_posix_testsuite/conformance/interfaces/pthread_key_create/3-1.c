/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_key_create()
 *
 * An optional destructor function may be associated with each key value.  At thread exit, if
 * a key value has a non-NULL destructor pointer, and the thread has a non-NULL value associated
 * with that key, the value of the key is set to NULL, and then the function pointed to is called
 * with the previously associated value as its sole argument.  The order of destructor calls is
 * unspecified if more than one destructor exists for a thread when it exits.
 *
 * Steps:
 * 1. Define an array of keys
 * 2. Use pthread_key_create() and create those keys
 * 3. Verify that you can set and get specific values for those keys without errors.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define KEY_VALUE 1000

pthread_key_t key;
int dest_cnt;

/* Destructor funciton */
void dest_func(void *p)
{
	dest_cnt++;
}

/* Thread function */
void *a_thread_func()
{

	/* Set the value of the key to a value */	
	if(pthread_setspecific(key, (void *)(KEY_VALUE)) != 0)
	{
		printf("Error: pthread_setspecific() failed\n");
		pthread_exit((void*) PTS_UNRESOLVED);
	}
	
	/* The thread ends here, the destructor for the key should now be called after this */
	pthread_exit(0);
}

int main()
{
	pthread_t new_th;

	/* Inialize the destructor flag */
	dest_cnt = 0;

	/* Create a key with a destructor function */
	if(pthread_key_create(&key, dest_func) != 0)
	{
		printf("Error: pthread_key_create() failed\n");
		pthread_exit((void*) PTS_UNRESOLVED);
	}

	/* Create a thread */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for the thread's return */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check if the destructor was called */
	if(dest_cnt == 0)
	{
		printf("Test FAILED: Destructor not called\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
