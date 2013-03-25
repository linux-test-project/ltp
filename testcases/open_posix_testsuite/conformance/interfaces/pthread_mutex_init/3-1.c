/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that the macro PTHREAD_MUTEX_INITIALIZER can be sued to intiailize
 * mutexes that are statically allocated.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

typedef struct my_data {
	pthread_mutex_t mutex;	/* Protects access to value */
	int value;		/* Access protected by mutex */
} my_data_t;

my_data_t data = { PTHREAD_MUTEX_INITIALIZER, 0 };

int main(void)
{
	printf("Test PASSED\n");
	return PTS_PASS;
}
