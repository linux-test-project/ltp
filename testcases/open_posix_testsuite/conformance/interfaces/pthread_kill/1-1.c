/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program verifies that the pthread_kill() function requests from the 
 kernel to deliver a specified signal to a specified thread.

 Using two global values of sem1 (INTHREAD and INMAIN), we are going to 
 control  when we want execution to be in main() and when we want 
 execution to be a_thread_func(). When the main() function sets sem1 to
 INTHREAD and keeps looping until sem1 gets changed back to INMAIN, the
 a_thread_func() will be exclusively running. Similarly, when the 
 a_thread_func() sets sem1 to INMAIN and keeps looping until sem1 gets
 changed back to INTHREAD, the main() function will be exclusively 
 running.

 Steps:
 1. From the main() function, create a new thread. Using the above 
    methodology, let the new thread run "exclusively."
 2. Inside the new thread, prepare for catching the signal indicated by
    SIGTOTEST, and calling a handler that sets handler_called to 1. Now 
    let the main() thread run "exclusively".
 3. Have main() send the signal indicated by SIGTOTEST to the new thread, 
    using pthread_kill(). Let the new thread continue running, 
    and from the main function, wait until handler_called is set to 
    something other than 0.
 4. In the new thread, if the handler wasn't called for more than 5 
    seconds, then set handler_called to -1, otherwise set it to 1.
 5. In either case, the main() function will continue execution and return 
    a PTS_PASS if handler_called was 1, and a PTS_FAIL if handler_called 
    was -1.
 */

#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define INTHREAD 0
# define INMAIN 1
# define SIGTOTEST SIGABRT

int sem1;		/* Manual semaphore */
int handler_called = 0;

void handler() {
	printf("signal was called\n");
	handler_called = 1;
	return;
}

void *a_thread_func()
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);

	sem1=INMAIN;

	while(sem1==INMAIN)
		sleep(1);

	sleep(5);

	handler_called=-1;
	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;

	sem1=INTHREAD;

	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	while(sem1==INTHREAD)
		sleep(1);

	if(pthread_kill(new_th, SIGTOTEST) != 0) 
	{
		printf("Test FAILED: Couldn't send signal to thread\n");
		return PTS_FAIL;
	}

	sem1=INTHREAD;
	
	while(handler_called==0)
		sleep(1);

	if(handler_called == -1) {
		printf("Test FAILED: Kill request timed out\n");
		return PTS_FAIL;
	} else if (handler_called == 0) {
		printf("Test FAILED: Thread did not recieve or handle\n");
		return PTS_FAIL;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


