/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Copyright (c) 2017, Richard Palethorpe <rpalethorpe@suse.com>
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that EBUSY is returned when pthread_cond_destroy() is called on a cond
 * var that has waiters. POSIX only recommends this behaviour, the required
 * behaviour is undefined.
 *
 * This test is very similar to pthread_barrier_destroy 2-1 which has more
 * explanation attached.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread(void *tmp)
{
	int rc = 0;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	rc = pthread_mutex_lock(&mutex);
	if (rc != 0) {
		perror("child: pthread_mutex_lock");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_cond_wait(&cond, &mutex);
	if (rc != 0) {
		perror("child: pthread_cond_wait");
		exit(PTS_UNSUPPORTED);
	}

	rc = pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		perror("child: pthread_mutex_unlock");
		exit(PTS_UNSUPPORTED);
	}

	return tmp;
}

void *watchdog(void *arg)
{
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	sleep(1);
	printf("watchdog: pthread_cond_destroy() appears to be blocking\n");
	if (pthread_cond_signal(&cond)) {
		perror("watchdog: pthread_cond_signal()");
		exit(PTS_UNRESOLVED);
	}

	return arg;
}

int main(void)
{
	pthread_t low_id, watchdog_thread;
	int rc = 0;

	rc = pthread_create(&low_id, NULL, thread, NULL);
	if (rc != 0) {
		perror("main: pthread_create");
		exit(PTS_UNRESOLVED);
	}

	sleep(1);

	rc = pthread_create(&watchdog_thread, NULL, watchdog, NULL);
	if (rc != 0) {
		perror("main: pthread_create");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_cond_destroy(&cond);
	if (rc != EBUSY) {
		printf("UNSUPPORTED: The standard recommends returning %d, EBUSY, but got %d, %s\n",
		       EBUSY, rc, strerror(rc));
		rc = PTS_UNSUPPORTED;
	} else {
		printf("PASSED: received EBUSY as per recommendation\n");
		rc = PTS_PASS;
	}

	pthread_cancel(watchdog_thread);
	pthread_cancel(low_id);

	exit(rc);
}
