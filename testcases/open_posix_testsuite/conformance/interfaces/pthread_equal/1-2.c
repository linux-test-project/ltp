/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_equal()
 * shall compare the thread ids t1 and t2.  The function shall return a non-zero
 * value if t1 and t2 are equal, othersise zero shall be returned.
 * No errors are defined.
 *
 * Steps:
 * 1.  Create 2 threads
 * 2.  Call pthread_equal and pass to it the two threads.
 *     They should not be equal.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

void *a_thread_func()
{

	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th1, new_th2;

	/* Create a new thread. */
	if (pthread_create(&new_th1, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Create another new thread. */
	if (pthread_create(&new_th2, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Call pthread_equal() and pass to it the 2 new threads.
	 * It should return a zero value, indicating that
	 * they are not equal. */
	if (pthread_equal(new_th1, new_th2) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	} else {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

}
