/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_attr_setstack()
 *
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr)
 * 2.  set the stacksize less tha PTHREAD_STACK_MIN
 */

#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "6-1"
#define FUNCTION "pthread_attr_setstack"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define STACKSIZE PTHREAD_STACK_MIN - sysconf(_SC_PAGE_SIZE)

static void *stack_addr;
size_t stack_size;

void *thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_attr_t attr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}

	/* Get the default stack_addr and stack_size value */
	rc = pthread_attr_getstack(&attr, &stack_addr, &stack_size);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_getstack");
		exit(PTS_UNRESOLVED);
	}
	/* printf("stack_addr = %p, stack_size = %u\n", stack_addr, stack_size); */

	stack_size = STACKSIZE;

	if (posix_memalign(&stack_addr, sysconf(_SC_PAGE_SIZE),
			   stack_size) != 0) {
		perror(ERROR_PREFIX "out of memory while "
		       "allocating the stack memory");
		exit(PTS_UNRESOLVED);
	}

	/* printf("stack_addr = %p, stack_size = %u\n", stack_addr, stack_size); */
	rc = pthread_attr_setstack(&attr, stack_addr, stack_size);
	if (rc != EINVAL) {
		perror(ERROR_PREFIX "Got the wrong return value");
		exit(PTS_FAIL);
	}

	rc = pthread_attr_destroy(&attr);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
