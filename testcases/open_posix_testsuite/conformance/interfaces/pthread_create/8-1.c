/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_create()
 * 
 * The signal state of the new thread will be initialized as so:
 *
 * - The signal mask shall be inherited from the created thread
 * - The set of signals pending for the new thread shall be empty.
 *
 * Steps:
 * 1.  In main(), create a signal mask with a few signals in the set (SIGUSR1 and SIGUSR2).
 * 2.  Raise those signals in main.  These signals now should be pending.
 * 3.  Create a thread using pthread_create().
 * 4.  The thread should have the same signal mask, but no signals should be pending.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

sigset_t th_pendingset, th_sigmask; 

void *a_thread_func()
{
	/* Obtain the signal mask of this thread. */
	pthread_sigmask(SIG_SETMASK, NULL, &th_sigmask);

	/* Obtain the pending signals of this thread. It should be empty. */
	if (sigpending(&th_pendingset) != 0) {
		printf("Error calling sigpending()\n");
		return (void *)PTS_UNRESOLVED;
	}
	
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;	
	sigset_t main_sigmask, main_pendingset;
	int ret;

	/* Empty set of signal mask and blocked signals */
	if ( (sigemptyset(&main_sigmask) != 0) || 
		(sigemptyset(&main_pendingset) != 0) )
	{
		perror("Error in sigemptyset()\n");
		return PTS_UNRESOLVED;
	}

	/* Add SIGCONT, SIGUSR1 and SIGUSR2 to the set of blocked signals */
	if (sigaddset(&main_sigmask, SIGUSR1) != 0)
	{
		perror("Error in sigaddset()\n");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&main_sigmask, SIGUSR2) != 0)
	{
		perror("Error in sigaddset()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Block those signals. */
	if (pthread_sigmask(SIG_SETMASK, &main_sigmask, NULL) != 0) 
	{
		printf("Error in pthread_sigmask()\n");
		return PTS_UNRESOLVED;
	}

	/* Raise those signals so they are now pending. */	
	if (raise(SIGUSR1) != 0) {
		printf("Could not raise SIGALRM\n");
		return -1;
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGALRM\n");
		return -1;
	}

	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait until the thread has finished execution. */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check to make sure that the sigmask of the thread is the same as the main thread */
	ret = sigismember(&th_sigmask, SIGUSR1);
	if(ret != 1)
	{
		if(ret == 0)
		{
			printf("Error: Thread did not inherit main()s signal mask. SIGUSR1 not a member of the signal set.\n");
			return PTS_FAIL;
		}

		perror("Error is sigismember()\n");
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_sigmask, SIGUSR2);
	if(ret != 1)
	{
		if(ret == 0)
		{
			printf("Test FAILED: Thread did not inherit main()s signal mask. SIGUSR2 not a member of the signal set.\n");
			return PTS_FAIL;
		}

		perror("Error is sigismember()\n");
		return PTS_UNRESOLVED;
	}

	/* Check to make sure that the pending set of the thread does not contain SIGUSR1 or
	 * SIGUSR2. */
	
	ret = sigismember(&th_pendingset, SIGUSR1);
	if(ret != 0)
	{
		if(ret == 1)
		{
			printf("Error: Thread did not inherit main()s signal mask. SIGUSR1 not a member of the signal set.\n");
			return PTS_FAIL;
		}

		perror("Error is sigismember()\n");
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_pendingset, SIGUSR2);
	if(ret != 0)
	{
		if(ret == 1)
		{
			printf("Test FAILED: Thread did not inherit main()s signal mask. SIGUSR2 not a member of the signal set.\n");
			return PTS_FAIL;
		}

		perror("Error is sigismember()\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}


