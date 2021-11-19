/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_key_create()
 *
 * Upon key creation, the value NULL shall be associated with the new key in all active threads.
 * Upon thread creation, the value NULL shall be associated with all defined keys in the new
 * thread.
 *
 * Steps:
 * 1. Create a key
 * 2. Verify that the default value is NULL
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	pthread_key_t key;
	void *rc;

	if (pthread_key_create(&key, NULL) != 0) {
		printf("Error: pthread_key_create() failed\n");
		return PTS_UNRESOLVED;
	} else {
		/* Verify that the value associated with "key" after it is newly created is
		 * NULL */
		rc = pthread_getspecific(key);
		if (rc != NULL) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
