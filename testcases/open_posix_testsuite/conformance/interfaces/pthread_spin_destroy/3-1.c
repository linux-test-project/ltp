/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_spin_destroy(pthread_spinlock_t *lock) may fail if:
 *
 * [EBUSY] The implementation has detected an attempt to 
 * initialize or destroy a spin lock while it is in use 
 * (for example, while being used in a pthread_spin_lock( )
 * call) by another thread.
 *
 * Note: This test will always pass 
 * 
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Create a child thread. The thread call pthread_spin_destroy() 
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;

static void* fn_chld(void *arg)
{ 
	int rc = 0;
	
	printf("child: destroy spin lock\n");
	rc = pthread_spin_destroy(&spinlock);
	if(rc == EBUSY)
	{
		printf("child: correctly got EBUSY\n");
		printf("Test PASSED\n");
	} 
	else
	{
		printf("child: got return code %d, %s\n", rc, strerror(rc));	
		printf("Test PASSED: *Note: Did not return EBUSY when destroying a spinlock already in use, but standard says 'may' fail\n");
	}
	exit(PTS_PASS);
}
 
int main()
{
	pthread_t child_thread;
	
	if(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0)
	{
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */	
	if(pthread_spin_lock(&spinlock) != 0)
	{
		printf("main cannot get spin lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	} 
	printf("main: acquired spin lock\n");
	
	printf("main: create thread\n");
	if(pthread_create(&child_thread, NULL, fn_chld, NULL) != 0)
	{
		printf("main: Error creating child thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to end execution */	
	pthread_join(child_thread, NULL);
	
	return PTS_PASS;
}
