/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutex_lock()
 *   shall lock the mutex object referenced by 'mutex'.  If the mutex is
 *   already locked, the calling thread shall block until the mutex becomes
 *   available. This operation shall return with the mutex object referenced
 *   by 'mutex' in the locked state with the calling thread as its owner.

 * Steps:
 *   -- Initialize a mutex to protect a global variable 'value'
 *   -- Create N threads. Each is looped M times to acquire the mutex,
 *      increase the value, and then release the mutex.
 *   -- Check if the value has increased properly (M*N); a broken mutex
 *      implementation may cause lost augments.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define    THREAD_NUM  	5
#define    LOOPS     	4

void *f1(void *parm);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int value;			/* value protected by mutex */

int main(void)
{
	int i, rc;
	pthread_attr_t pta;
	pthread_t threads[THREAD_NUM];
	//pthread_t           self = pthread_self();

	pthread_attr_init(&pta);
	pthread_attr_setdetachstate(&pta, PTHREAD_CREATE_JOINABLE);

	/* Create threads */
	fprintf(stderr, "Creating %d threads\n", THREAD_NUM);
	for (i = 0; i < THREAD_NUM; ++i)
		rc = pthread_create(&threads[i], &pta, f1, NULL);

	/* Wait to join all threads */
	for (i = 0; i < THREAD_NUM; ++i)
		pthread_join(threads[i], NULL);
	pthread_attr_destroy(&pta);
	pthread_mutex_destroy(&mutex);

	/* Check if the final value is as expected */
	if (value != (THREAD_NUM) * LOOPS) {
		fprintf(stderr, "Using %d threads and each loops %d times\n",
			THREAD_NUM, LOOPS);
		fprintf(stderr, "Final value must be %d instead of %d\n",
			(THREAD_NUM) * LOOPS, value);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

void *f1(void *parm LTP_ATTRIBUTE_UNUSED)
{
	int i, tmp;
	int rc = 0;
	pthread_t self = pthread_self();

	/* Loopd M times to acquire the mutex, increase the value,
	   and then release the mutex. */

	struct timespec lock_wait_ts = {0, 1000000};

	for (i = 0; i < LOOPS; ++i) {
		rc = pthread_mutex_lock(&mutex);
		if (rc != 0) {
			fprintf(stderr,
				"Error on pthread_mutex_lock(), rc=%d\n", rc);
			return (void *)(PTS_FAIL);
		}

		tmp = value;
		tmp = tmp + 1;
		fprintf(stderr, "Thread(0x%p) holds the mutex\n", (void *)self);
		nanosleep(&lock_wait_ts, NULL);	/* delay the increasement operation */
		value = tmp;

		rc = pthread_mutex_unlock(&mutex);
		if (rc != 0) {
			fprintf(stderr,
				"Error on pthread_mutex_unlock(), rc=%d\n", rc);
			return (void *)(PTS_UNRESOLVED);
		}
		sleep(1);
	}
	pthread_exit(0);
	return (void *)(0);
}
