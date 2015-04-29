/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that pthread_key_create()
 *
 * If successful, the pthread_key_create() function shall store the newly
 * created key value at *key and shall return zero.  Otherwise, an error number
 * shall be returned to indicate an error:
 *
 * [EAGAIN] - the system lacked the necessary resources to create another
 *            thread_specific data key, or the system imposed limit on the
 *            total number of keys per process [PTHREAD_KEYS_MAX] has been
 *            exceeded.
 *
 * [ENOMEM] - insufficient memory exists to create the key.
 *
 * TESTING [EAGAIN]
 *
 * Steps:
 * 1. Define an array of keys
 * 2. Use pthread_key_create() and create those keys
 * 3. Verify that you can set and get specific values for those keys without
 *    errors.
 *
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_key_t keys[PTHREAD_KEYS_MAX + 1];

int main(void)
{
	int i, rc;

	for (i = 0; i <= PTHREAD_KEYS_MAX; i++) {
		rc = pthread_key_create(&keys[i], NULL);

		if (rc != 0)
			break;
	}

	if (i == PTHREAD_KEYS_MAX) {
		if (rc == EAGAIN) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Expected EAGAIN on exceeding the limit, got: %d\n", rc);
			return PTS_FAIL;
		}
	}

	if (rc == EAGAIN) {
		printf("EAGAIN returned before the key limit was exceeded\n");
		return PTS_FAIL;
	}

	printf("Error: pthread_key_create() failed with %d\n", rc);
	return PTS_UNRESOLVED;
}
