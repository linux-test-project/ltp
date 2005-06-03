/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * Test pthread_rwlock_timedrdlock(pthread_rwlock_t * rwlock)
 * 
 * If a signal that causes a signal handler to be executed is delivered to 
 * a thread blocked on a read-write lock via a call to pthread_rwlock_timedrdlock( ),
 * upon return from the signal handler the thread shall resume waiting for the lock 
 * as if it was not interrupted.
 *
 * Steps:
 * 1. main thread  create read-write lock 'rwlock', and lock it for writing
 * 2. main thread create a thread sig_thread, the thread is set to handle SIGUSR1
 * 3. sig_thread timed lock 'rwlock' for reading, but blocked
 * 4. While the sig_thread is waiting(not expried yet), main thread send SIGUSR1 
 *    to sig_thread via pthread_kill
 * 5. test that thread handler is called, inside the handler, make the thread sleep
 *    for a period that the specified 'timeout' for pthread_rwlock_timedrdlock() 
 *    should have expired (timeout * 2) 
 * 6. While sig_thread sleeping in signal handler, main thread unlock 'rwlock' 
 * 7. check that when thread handler returns, sig_thread get the read lock without 
 *    getting ETIMEDOUT.
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

/* thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 2

static pthread_t sig_thread;
static pthread_rwlock_t rwlock;

static int thread_state;
static int handler_state;
static int expired;
static struct timeval before_wait, after_wait;

static void sig_handler() {

	struct timespec sleep_time_req;
	
	sleep_time_req.tv_sec = TIMEOUT*2;
	sleep_time_req.tv_nsec = 0;
	
	if(pthread_equal(pthread_self(), sig_thread))
	{
		printf("sig_handler: signal is handled by thread\n");
		/* sig_handler will not sleep 2 times more than the timeout for the
		 * pthread_rwlock_timerdlock is waiting for */
		printf("sig_handler: sleeping for %d seconds\n", (int)sleep_time_req.tv_sec);	
		handler_state = 2;
		sleep((int)sleep_time_req.tv_sec);
	}
	else
	{
		printf("sig_handler: signal is not handled by thread\n");
		exit(PTS_UNRESOLVED);
	}

	handler_state = 3;
}

static void * th_fn(void *arg)
{
	struct sigaction act;
	struct timespec abs_timeout;
	int rc;
	handler_state = 2;
	expired = 0;

	/* Set up handler for SIGUSR1 */
	
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	/* block all the signal when hanlding SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);
	
	gettimeofday(&before_wait, NULL);
	abs_timeout.tv_sec = before_wait.tv_sec + TIMEOUT;
	abs_timeout.tv_nsec = before_wait.tv_usec * 1000;
	
	thread_state = ENTERED_THREAD;
	
	printf("thread: attempt timed read lock, %d seconds\n", TIMEOUT);
	rc = pthread_rwlock_timedrdlock(&rwlock, &abs_timeout);
	if(rc == 0)
	{
		printf("thread: correctly acquired read lock\n");
		expired = 0;	
	}	 
	else if(rc == ETIMEDOUT)
	{
		printf("thread: timer expired, did not acquire read lock");
		expired = 1;
	}
	else
	{
		printf("Error at pthread_rwlock_timedrdlock()");
		exit(PTS_UNRESOLVED);
	}

	gettimeofday(&after_wait, NULL);

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main()
{
	int cnt;

	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&rwlock) != 0)
	{
		printf("pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&sig_thread, NULL, th_fn, NULL) != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* wait for the thread to get ready for handling signal */	
	cnt = 0;
	do{
		sleep(1);
	}while(thread_state != ENTERED_THREAD && cnt++ < TIMEOUT);
	
	if(thread_state != ENTERED_THREAD)
	{
		printf("Error: thread did not block when getting read lock\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: fire SIGUSR1 to thread\n");
	if(pthread_kill(sig_thread, SIGUSR1) != 0)
	{
		printf("Error in pthread_kill()");
		exit(PTS_UNRESOLVED);
	}
	
	/* Wait for signal handler to sleep so that main can unlock the rwlock while
	 * it is sleeping. (this way, the rwlock will be unlocked when the signal handler
	 * returns, and control is given back to the thread) */

	cnt = 0;
	do{
		sleep(TIMEOUT);
	}while(handler_state !=2 && cnt++ < 2);

	if(handler_state == 1)
	{
		printf("Error: signal handler did not get called\n");
		exit(PTS_UNRESOLVED);
	}
	else if(handler_state == 3)
	{
		printf("Error: signal handler incorrectly exited\n");
		exit(PTS_UNRESOLVED);	
	}

	if(expired == 1)
	{
		printf("Error: thread timeout in sig_handler\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&rwlock) != 0)
	{
		printf("Error at pthread_rwlock_unlock()\n");
		exit(PTS_UNRESOLVED);
	}
	
	/* wait at most 4*TIMEOUT seconds for thread to exit */
	cnt = 0;
	do{
		sleep(1);
	}while(thread_state != EXITING_THREAD && cnt++ < 4*TIMEOUT);
	
	
	if(cnt >= 4*TIMEOUT)
	{
		/* thread blocked*/
		printf("Test FAILED: thread blocked even afer the abs_timeout expires\n");
		exit(PTS_FAIL);		
	}
	
	if(expired == 1)
	{
		printf("Test FAILED: thread should get the read lock\n");
		exit(PTS_FAIL);
	}
	
	if(pthread_join(sig_thread, NULL) != 0)
	{
		printf("Error at pthread_join()");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("Error at pthread_destroy()");
		exit(PTS_UNRESOLVED);
	}
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}
