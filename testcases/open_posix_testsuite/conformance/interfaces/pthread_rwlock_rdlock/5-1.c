/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
 *
 *	A thread may hold multiple concurrent read locks on 'rwlock' and the application shall
 *	ensure that the thread will perform matching unlocks for each read lock.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'rwlock' with pthread_rwlock_init()
 * 2.  Main read locks 'rwlock' 10 times
 * 3.  Main unlocks 'rwlock' 10 times.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define COUNT 10

int main(void)
{

	static pthread_rwlock_t rwlock;
	int i;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < COUNT; i++) {
		if (pthread_rwlock_rdlock(&rwlock) != 0) {
			printf
			    ("Test FAILED: main cannot get read-lock rwlock number: %d\n",
			     i);
			return PTS_FAIL;
		}
	}

	for (i = 0; i < COUNT; i++) {
		if (pthread_rwlock_unlock(&rwlock) != 0) {
			printf
			    ("Test FAILED: main cannot unlock rwlock number %d",
			     i);
			return PTS_FAIL;
		}
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
