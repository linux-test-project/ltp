/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrier_wait()
 *
 * When the required number of threads have called pthread_barrier_wait( ) 
 * specifying the barrier, the constant PTHREAD_BARRIER_SERIAL_THREAD shall 
 * be returned to one unspecified thread and zero shall be returned 
 * to each of the remaining threads. At this point, the barrier shall
 * be reset to the state it had as a result of the most recent 
 * pthread_barrier_init( ) function that referenced it.
 * 
 * Steps:
 * 1. Main thread do the following for LOOP_NUM times
 * 2. In each loop, Main thread initialize barrier, with count set to THREAD_NUM
 * 3. Main create THREAD_NUM threads
 * 4. Each thread will call pthread_barrier_wait()
 * 5. When the last thread calls pthread_barrier_wait, only one thread will 
 *    get PTHREAD_BARRIER_SERIAL_THREAD, all the other threads should get zero
 * 6. This holds true for every loop.
 * 
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

#define THREAD_NUM 5 
#define LOOP_NUM 3 

static pthread_barrier_t barrier;
static int serial;
static int normal_rt;

static void* fn_chld(void *arg)
{ 
	int rc = 0;
	int thread_num = *(int*)arg;
	
	printf("child[%d]: barrier wait\n", thread_num);
	rc = pthread_barrier_wait(&barrier);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
	{
		printf("Test FAILED: child[%d]: pthread_barrier_wait() get unexpected "
			"return code : %d\n" , thread_num, rc);
		exit(PTS_FAIL);
	} 
	else if(rc == PTHREAD_BARRIER_SERIAL_THREAD)
	{
		serial++;
		printf("child[%d]: get PTHREAD_BARRIER_SERIAL_THREAD\n", thread_num);
	}
	else
	{
		normal_rt++;
	}
	
	pthread_exit(0);
	return NULL;
}
 
int main()
{
	pthread_t child_threads[THREAD_NUM];
	int cnt;
	int loop;

	printf("Initialize barrier with count = %d\n", THREAD_NUM);
	if(pthread_barrier_init(&barrier, NULL, THREAD_NUM) != 0)
	{
		printf("main: Error at pthread_barrier_init()\n");
		return PTS_UNRESOLVED;
	}

	for(loop = 0; loop < LOOP_NUM; loop++)
	{
		serial = 0;
		normal_rt = 0;
		printf("\n-Loop %d-\n", loop);

		printf("main: create %d child threads\n", THREAD_NUM);
		for(cnt = 0; cnt < THREAD_NUM; cnt++)
		{
			if(pthread_create(&child_threads[cnt], NULL, fn_chld, &cnt) != 0)
			{
				printf("main: Error at %dth pthread_create()\n", cnt);
				return PTS_UNRESOLVED;
			}
	
		}
		printf("main: wait for child threads to end\n");
		for(cnt = 0; cnt < THREAD_NUM; cnt++)
		{	
			if(pthread_join(child_threads[cnt], NULL) != 0)
			{
				printf("main: Error at %dth pthread_join()\n", cnt);
				exit(PTS_UNRESOLVED);
			}
		}
		
		if(serial != 1 || (serial + normal_rt) != THREAD_NUM )
		{
			printf("Test FAILED: On %d loop, PTHREAD_BARRIER_SERIAL_THREAD "
				"should be returned to one unspecified thread\n", loop);
			return PTS_FAIL;
		}
		
	}

	if(pthread_barrier_destroy(&barrier) != 0)
	{
		printf("Error at pthread_barrier_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("\nTest PASSED\n");
	return PTS_PASS;
}
