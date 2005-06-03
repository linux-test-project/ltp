/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * Test pthread_rwlock_timedwrlock(pthread_rwlock_t * rwlock)
 * 
 * If a signal that causes a signal handler to be executed is delivered to 
 * a thread blocked on a read-write lock via a call to pthread_rwlock_timedwrlock( ),
 * upon return from the signal handler the thread shall resume waiting for the lock 
 * as if it was not interrupted.
 *
 * Test that after returning from a signal handler, the reader will continue
 * to wait with timedrdlock as long as the specified 'timeout' does not expire (the 
 * time spent in signal handler is longer than the specifed 'timeout').
 *
 * Steps:
 * 1. main thread  create and write lock 'rwlock'
 * 2. main thread create a thread sig_thread, the thread is set to handle SIGUSR1
 * 3. sig_thread timed write-lock 'rwlock' for writing, it should block
 * 4. While the sig_thread is waiting (not expired yet), main thread sends SIGUSR1 
 *    to sig_thread via pthread_kill
 * 5. Check that when thread handler returns, sig_thread resume block
 * 7. When the wait is terminated, check that the thread wait for a proper period before
 *    expiring. 
 *
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "posixtest.h"

static pthread_t sig_thread;
static pthread_rwlock_t rwlock;

static int thread_state;
static int handler_called;
static struct timeval before_wait, after_wait;


/* thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 5

/* Signal handler called by the thread when SIGUSR1 is received */
static void sig_handler() {

	if(pthread_equal(pthread_self(), sig_thread))
	{
		printf("sig_handler: signal is handled by sig_thread\n");
		handler_called = 1;
		
	}
	else
	{
		printf("sig_handler: signal is not handled by sig_thread\n");
		exit(PTS_UNRESOLVED);
	}
}

static void * th_fn(void *arg)
{
	struct sigaction act;
	struct timespec abs_timeout;
	int rc = 0;
	
	handler_called = 0;

	/* Set up signal handler for SIGUSR1 */	

	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	/* block all the signal when hanlding SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);
	
	gettimeofday(&before_wait, NULL);
	abs_timeout.tv_sec = before_wait.tv_sec + TIMEOUT;
	abs_timeout.tv_nsec = before_wait.tv_usec * 1000;
	
	printf("thread: attempt timed write lock, %d seconds\n", TIMEOUT);
	thread_state = ENTERED_THREAD;
	rc = pthread_rwlock_timedwrlock(&rwlock, &abs_timeout);
	if(rc != ETIMEDOUT)
	{
		printf("sig_thread: pthread_rwlock_timedwrlock returns %d\n", rc);
		exit(PTS_FAIL);
	}
 	printf("thread: timer correctly expired\n");
	gettimeofday(&after_wait, NULL);

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main()
{
	int cnt;
	struct timeval time_diff;

	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&sig_thread, NULL, th_fn, NULL) != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for the thread to get ready for handling signal (the thread should
	 * be block on rwlock since main() has the write lock at this point) */	
	cnt = 0;
	do{
		sleep(1);
	}while(thread_state != ENTERED_THREAD && cnt++ < TIMEOUT);
	
	if(thread_state != ENTERED_THREAD)
	{
		printf("Unexpected thread state %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf("main: fire SIGUSR1 to thread\n");
	if(pthread_kill(sig_thread, SIGUSR1) != 0)
	{
		printf("main: Error at pthread_kill()\n");
		exit(PTS_UNRESOLVED);
	}
	
	/* wait at most 2*TIMEOUT seconds */
	cnt = 0;
	do{
		sleep(1);
	}while(thread_state != EXITING_THREAD && cnt++ < 2*TIMEOUT);
	
	if(cnt >= 2*TIMEOUT)
	{
		/* thread blocked*/
		printf("Test FAILED: thread blocked even afer the abs_timeout expired\n");
		exit(PTS_FAIL);		
	}
	
	if(handler_called != 1)
	{
		printf("The handler for SIGUSR1 did not get called\n");
		exit(PTS_UNRESOLVED);
	}	
	
	/* Test that the thread block for the correct TIMOUT time */
	time_diff.tv_sec = after_wait.tv_sec - before_wait.tv_sec;
	time_diff.tv_usec = after_wait.tv_usec - before_wait.tv_usec;
	if (time_diff.tv_usec < 0)
	{
		--time_diff.tv_sec;
		time_diff.tv_usec += 1000000;
	}
	if(time_diff.tv_sec < TIMEOUT)
	{
		printf("Test FAILED: Timeout was for %d seconds, but waited for %ld.%06ld seconds instead\n",
			TIMEOUT, (long)time_diff.tv_sec, (long)time_diff.tv_usec);
		exit(PTS_FAIL);
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(sig_thread, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_destroy()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


