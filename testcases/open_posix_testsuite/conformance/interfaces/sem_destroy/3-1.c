/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  majid.awad REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

/*
 * Test case verifies sem_destroy shall destroy on initialized semaphore
 * upon which no threads are currently blocked.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_destroy"

static sem_t psem, csem;
static int n;
static void *producer(void *);
static void *consumer(void *);

int main(void)
{
	pthread_t prod, cons;
	int err;
	long cnt = 3;

	n = 0;
	if (sem_init(&csem, 0, 0) < 0) {
		perror("sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_init(&psem, 0, 1) < 0) {
		perror("sem_init");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&prod, NULL, producer, (void *)cnt) != 0) {
		perror("pthread_create");
		return PTS_UNRESOLVED;
	}

	if (pthread_create(&cons, NULL, consumer, (void *)cnt) != 0) {
		perror("pthread_create");
		return PTS_UNRESOLVED;
	}

	err = pthread_join(prod, NULL);
	if (err) {
		printf("Failed to join thread: %s", strerror(err));
		return PTS_UNRESOLVED;
	}

	err = pthread_join(cons, NULL);
	if (err) {
		printf("Failed to join thread: %s", strerror(err));
		return PTS_UNRESOLVED;
	}

	if (sem_destroy(&psem) == 0 && sem_destroy(&csem) == 0) {
		puts("TEST PASS");
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}

static void *producer(void *arg)
{
	int i, cnt;
	cnt = (long)arg;
	for (i = 0; i < cnt; i++) {
		sem_wait(&psem);
		n++;
		sem_post(&csem);
	}
	return NULL;
}

static void *consumer(void *arg)
{
	int i, cnt;
	cnt = (long)arg;
	for (i = 0; i < cnt; i++) {
		sem_wait(&csem);
		sem_post(&psem);
	}
	return NULL;
}
