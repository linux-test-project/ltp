/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * 
 * Test pthread_spin_init(pthread_spinlock_t * lock, int pshared)
 *
 * These functions may fail if:
 * 	[EBUSY] The implementation has detected an attempt to initialize or destroy a spin
 *	lock while it is in use (for example, while being used in a pthread_spin_lock( )
 *	call) by another thread.
 *
 * Note: This case will always pass.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Main create a child thread
 * 4.  Child thread initialize the spin lock when main holds the lock
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
static int pshared;

static void* fn_chld(void* arg)
{
	int rc;
	/* child: initialize a spin lock being locked by main thread */
	printf("child: attempt initialize spin lock\n");
	rc = pthread_spin_init(&spinlock, pshared);
	if(rc == EBUSY)
		printf("child: correctly got EBUSY\n");
	else
	{
		printf("child: got return code %d, %s\n", rc, strerror(rc));
		printf("Test PASSED: *Note: Did not return EBUSY when initializing a spinlock already in use, but standard says 'may' fail\n");
	}
	exit(PTS_PASS);
}
 
int main()
{
	int rc = 0;
	pthread_t child_thread;

	#ifdef PTHREAD_PROCESS_PRIVATE
		pshared = PTHREAD_PROCESS_PRIVATE;
	#else
 		pshared = -1;
	#endif

	printf("main: initialize spin lock\n");
	
	rc = pthread_spin_init(&spinlock, pshared);
	if(rc != 0)
	{
		printf("Test FAILED:  Error at pthread_spin_init()\n");
		return PTS_FAIL;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */	
	if(pthread_spin_lock(&spinlock) != 0)
	{
		printf("Error: main cannot get spin lock when no one owns the lock\n");
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
