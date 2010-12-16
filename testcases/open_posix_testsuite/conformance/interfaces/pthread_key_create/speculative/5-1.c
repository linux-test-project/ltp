/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_key_create()
 *
 * If successful, the pthread_key_create() function shall store the newly created key value
 * at *key and shall return zero.  Otherwise, an error number shall be returned to indicate
 * an error:
 *
 * [EAGAIN] - the system lacked the necessary resources to create another thread_specific
 *            data key, or the system imposed limit on the total number of keys per process
 *            [PTHREAD_KEYS_MAX] has been exceeded.
 *
 * [ENOMEM] - insufficient memory exists to create the key.
 *
 * TESTING [EAGAIN]
 *
 * Steps:
 * 1. Define an array of keys
 * 2. Use pthread_key_create() and create those keys
 * 3. Verify that you can set and get specific values for those keys without errors.
 *
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

pthread_key_t keys[PTHREAD_KEYS_MAX];

int main()
{
	int i, rc;

	for (i = 0; i<= PTHREAD_KEYS_MAX; i++)
	{
		rc = pthread_key_create(&keys[i], NULL);
		pthread_key_t key;
		rc = pthread_key_create(&key, NULL);
		if (i == PTHREAD_KEYS_MAX)
		{
			if (rc != EAGAIN)
			{
				printf("Test FAILED: Expected EAGAIN when exceeded the limit of keys in a single process, but got: %d\n", rc);
				return PTS_FAIL;
			}
		}

		else if (rc != 0)
		{
			if (rc != EAGAIN)
			{
				printf("Error: pthread_key_create() failed\n");
				return PTS_UNRESOLVED;
			} else
			{
				printf("Test FAILED: EAGAIN was returned before the key limit was exceeded\n");
				return PTS_FAIL;
			}
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}