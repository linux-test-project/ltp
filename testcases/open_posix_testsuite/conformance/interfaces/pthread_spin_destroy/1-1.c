/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * 
 * Test pthread_spin_destroy(pthread_spinlock_t * lock)
 *
 * pthread_spin_destroy( ) function shall destroy the spin lock 
 * referenced by lock and release any resources used by the lock.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with 
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Main thread unlock 'spinlock'
 * 4.  Main thread destroy the 'spinlock'
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
 
int main()
{
	int rc = 0;

	printf("main: initialize spin lock\n");
	if(pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0)
	{
		printf("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */	
	if(pthread_spin_lock(&spinlock) != 0)
	{
		printf("Unresolved: main cannot get spin lock when no one owns the lock\n");
		return PTS_UNRESOLVED;
	} 
	
	printf("main: acquired spin lock\n");
	
	printf("main: unlock spin lock\n");	
	if(pthread_spin_unlock(&spinlock) != 0)
	{
		printf("main: Error at pthread_spin_unlock()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: destroy spin lock\n");
	rc = pthread_spin_destroy(&spinlock);
	if(rc != 0)
	{
		printf("Test FAILED: Error at pthread_spin_destroy()"
			"Return code : %d\n", rc);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
