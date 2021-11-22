/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that the macro PTHREAD_COND_INITIALIZER can be used to intiailize
 * condition variables that are statically allocated.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

static pthread_cond_t cond PTS_ATTRIBUTE_UNUSED = PTHREAD_COND_INITIALIZER;

int main(void)
{
	printf("Test PASSED\n");
	return PTS_PASS;
}
