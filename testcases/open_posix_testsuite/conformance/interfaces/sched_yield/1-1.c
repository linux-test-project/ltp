/* 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that the running thread relinquish the processor until it again becomes
 * the head of its thread list.
 *
 * This test launch NB_THREAD threads. All threads call sched_yield LOOP times.
 * At the return of sched_yield, the thread checks that another thread has
 * changed the value of nb_call.
 */
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include "posixtest.h"

#define NB_THREAD 5 /* Shall be >= 2 */
#define LOOP 3      /* Shall be >= 1 */


int nb_call = 0;

void * runner(void * arg) {
	int i=0, nc, result = 0;
	
	for(;i<LOOP;i++){
		nc = ++nb_call;
		sched_yield();

		/* If the value of nb_call has not change since the last call
		   of sched_yield, that means that the thread does not 
		   relinquish the processor */
		if(nc == nb_call) {
			result++;
		}
	}
	++nb_call;
	pthread_exit((void*)(&result));
	
	return NULL;
}

int main() {
	int i;
	pthread_t tid[NB_THREAD];
	int *tmpresult, result = 0;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	for(i=0; i<NB_THREAD; i++) {
		if(pthread_create(tid+i, &attr, runner, NULL) != 0) {
			perror("An error occurs when calling pthread_create()");
			return PTS_UNRESOLVED;
		}
	}
	
	for(i=0; i<NB_THREAD; i++) {
		if(pthread_join(tid[i], (void**) &tmpresult) != 0) {
			perror("An error occurs when calling pthread_join()");
			return PTS_UNRESOLVED;
		}
		result += *tmpresult;
	}

	switch(result){
	case 0: 
		printf("Test PASSED\n");
		return PTS_PASS;
	default: 
		printf("A thread does not relinquish the processor.\n");
		return PTS_FAIL;
	}
		
}
