/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_cond_timedwait()
 *   Upon successful completion, a value of zero shall be returned.
 */
 
#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "posixtest.h"

#define INTERVAL  1
#define TIMEOUT   5

struct testdata
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
} td;

int t1_start = 0;
int signaled = 0;

void *t1_func(void *arg)
{
	int rc;
	struct timespec timeout;
	struct timeval  curtime;
	
	if (pthread_mutex_lock(&td.mutex) != 0) {
		fprintf(stderr,"Thread1 failed to acquire the mutex\n");
		exit(PTS_UNRESOLVED);
	}
	fprintf(stderr,"Thread1 started\n");
	t1_start = 1;	/* let main thread continue */

	if (gettimeofday(&curtime, NULL) !=0 ) {
		fprintf(stderr,"Fail to get current time\n");
		exit(PTS_UNRESOLVED);
	}
	timeout.tv_sec = curtime.tv_sec + TIMEOUT;
	timeout.tv_nsec = curtime.tv_usec * 1000;

	fprintf(stderr,"Thread1 is waiting for the cond\n");
	rc = pthread_cond_timedwait(&td.cond, &td.mutex, &timeout);
	if (rc != 0) {
		if (rc == ETIMEDOUT) {
			fprintf(stderr,"Thread1 stops waiting when time is out\n");
			exit(PTS_UNRESOLVED);
		}
		else {
			fprintf(stderr,"pthread_cond_timedwait return %d\n", rc);
	                printf("Test FAILED\n");
			exit(PTS_FAIL);
                }
	}
	
	if (signaled == 0) {
		fprintf(stderr,"Thread1 did not block on the cond at all\n");
                exit(PTS_UNRESOLVED);
	}
	fprintf(stderr,"Thread1 wakened and got returned value 0\n");
	
	if (pthread_mutex_unlock(&td.mutex) != 0) {
		fprintf(stderr,"Thread1 failed to release the mutex\n");
                exit(PTS_UNRESOLVED);
	}
	fprintf(stderr,"Thread1 released the mutex\n");
	return NULL;
}

int main()
{
	pthread_t  thread1;

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
	sleep(INTERVAL);
	
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
