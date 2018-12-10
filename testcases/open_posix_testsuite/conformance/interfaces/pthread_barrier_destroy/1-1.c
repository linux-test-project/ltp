/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrier_destroy()
 *
 *
 * The pthread_barrier_destroy() function shall destroy the barrier
 * referenced by barrier and release any resources used by the barrier.
 *
 * Steps:
 * 1. Main initialize barrier with count 2
 * 2. Main destroy the barrier
 * 3. Repeat step 1,2 for N times
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

static pthread_barrier_t barrier;
#define LOOP_NUM 5

int main(void)
{
	int cnt;
	int rc;

	for (cnt = 0; cnt < LOOP_NUM; cnt++) {
		if (pthread_barrier_init(&barrier, NULL, 2) != 0) {
			printf
			    ("Test FAILED: Error at pthread_barrier_init()\n");
			return PTS_FAIL;
		}

		rc = pthread_barrier_destroy(&barrier);
		if (rc != 0) {
			printf
			    ("Test FAILED: Error at pthread_barrier_destroy() "
			     " return code: %d, %s\n", rc, strerror(rc));
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
