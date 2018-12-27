/*
 * Copyright (c) 2018, Linux Test Project
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Cancellation steps happen asynchronously with respect to
 * the pthread_cancel(). The return status of pthread_cancel()
 * merely informs the caller whether the cancellation request
 * was successfully queued.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"
#include "safe_helpers.h"

#define TIMEOUT_MS 5000
#define SLEEP_MS 1

static volatile int after_cancel;
static volatile int thread_sleep_time;
static sem_t sem;

static void cleanup_func(LTP_ATTRIBUTE_UNUSED void *unused)
{
	struct timespec cleanup_ts = {0, SLEEP_MS*1000000};
	do {
		nanosleep(&cleanup_ts, NULL);
		thread_sleep_time += SLEEP_MS;
	} while (after_cancel == 0 && thread_sleep_time < TIMEOUT_MS);
}

static void *thread_func(LTP_ATTRIBUTE_UNUSED void *unused)
{
	int waited_for_cancel_ms = 0;
	struct timespec cancel_wait_ts = {0, SLEEP_MS*1000000};

	SAFE_PFUNC(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL));
	pthread_cleanup_push(cleanup_func, NULL);

	SAFE_FUNC(sem_post(&sem));
	while (waited_for_cancel_ms < TIMEOUT_MS) {
		nanosleep(&cancel_wait_ts, NULL);
		waited_for_cancel_ms += SLEEP_MS;
	}

	/* shouldn't be reached */
	printf("Error: cancel never arrived\n");
	pthread_cleanup_pop(0);
	exit(PTS_FAIL);
	return NULL;
}

int main(void)
{
	pthread_t th;

	SAFE_FUNC(sem_init(&sem, 0, 0));
	SAFE_PFUNC(pthread_create(&th, NULL, thread_func, NULL));

	/* wait for thread to start */
	SAFE_FUNC(sem_wait(&sem));
	SAFE_PFUNC(pthread_cancel(th));

	/*
	 * if cancel action would run synchronously then
	 * thread will sleep for too long, because it
	 * would never see after_cancel == 1
	 */
	after_cancel = 1;

	SAFE_PFUNC(pthread_join(th, NULL));

	if (thread_sleep_time >= TIMEOUT_MS) {
		printf("Error: cleanup_func hit timeout\n");
		exit(PTS_FAIL);
	}

	if (thread_sleep_time == 0) {
		printf("Error: cleanup_func never called\n");
		exit(PTS_FAIL);
	}

	printf("Thread cancelled after %d ms.\n", thread_sleep_time);
	printf("Test PASSED\n");
	exit(PTS_PASS);
}
