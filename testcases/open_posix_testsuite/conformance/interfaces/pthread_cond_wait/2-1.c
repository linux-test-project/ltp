/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_wait()
 *   Upon successful return, the mutex shall have been locked and shall be owned 
 *   by the calling thread.
 */
 
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

struct testdata
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
} td;

pthread_t  thread1;

int t1_start = 0;
int signaled = 0;

/* Alarm handler */
void alarm_handler(int signo)
{
	printf("Error: failed to wakeup thread\n");
	pthread_cancel(thread1); 
	exit(PTS_UNRESOLVED);
}

void *t1_func(void *arg)
{
	int rc;
	
	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr,"Thread1 failed to acquire the mutex\n");
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr,"Thread1 started\n");
	t1_start = 1;	/* let main thread continue */
	
	fprintf(stderr,"Thread1 is waiting for the cond\n");
	rc = pthread_cond_wait(&td.cond, &td.mutex);
	if(rc != 0) {
		fprintf(stderr,"pthread_cond_wait return %d\n", rc);
                exit(PTS_UNRESOLVED);
	}
	
	fprintf(stderr,"Thread1 wakened\n");
	if(signaled == 0) {
		fprintf(stderr,"Thread1 did not block on the cond at all\n");
                exit(PTS_UNRESOLVED);
	}
	
	if (pthread_mutex_trylock(&td.mutex) == 0) {
		fprintf(stderr,"Thread1 should not be able to lock the mutex again\n");
                printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
	fprintf(stderr,"Thread1 failed to trylock the mutex (as expected)\n");

	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr,"Thread1 failed to release the mutex\n");
                printf("Test FAILED\n");
		exit(PTS_FAIL);
	}
	fprintf(stderr,"Thread1 released the mutex\n");
	return NULL;
}

int main()
{

	struct sigaction act;

	if (pthread_mutex_init(&td.mutex, NULL) != 0) {
		fprintf(stderr,"Fail to initialize mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_cond_init(&td.cond, NULL) != 0) {
		fprintf(stderr,"Fail to initialize cond\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&thread1, NULL, t1_func, NULL) != 0) {
		fprintf(stderr,"Fail to create thread 1\n");
		return PTS_UNRESOLVED;
	}
	while(!t1_start)	/* wait for thread1 started */
		usleep(100);
	
	/* acquire the mutex released by pthread_cond_wait() within thread 1 */
	if (pthread_mutex_lock(&td.mutex) != 0) {	
		fprintf(stderr,"Main failed to acquire mutex\n");
		return PTS_UNRESOLVED;
	}
	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr,"Main failed to release mutex\n");
		return PTS_UNRESOLVED;
	}
	sleep(1);
	
	/* Setup alarm handler */
	act.sa_handler=alarm_handler;
	act.sa_flags=0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	alarm(5);

	fprintf(stderr,"Time to wake up thread1 by signaling a condition\n");
	signaled = 1;
	if (pthread_cond_signal(&td.cond) != 0) {
		fprintf(stderr,"Main failed to signal the condition\n");
		return PTS_UNRESOLVED;
	}
	
	pthread_join(thread1, NULL);
	printf("Test PASSED\n");
	return PTS_PASS;
}


