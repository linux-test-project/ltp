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
 * 2.  set the stackaddr and stacksize to attr
 * 3.  create a thread with the attr
 * 4.  In the created thread, read the stacksize and stackaddr
 */

/* For pthread_getattr_np(3) -- not a POSIX compliant API. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/param.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "pthread_attr_setstack"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

static void *stack_addr;
size_t stack_size;

void *thread_func()
{
	pthread_attr_t attr;
	void *saddr;
	size_t ssize;
	int rc;

	if ((rc = pthread_getattr_np(pthread_self(), &attr)) != 0) {
		printf(ERROR_PREFIX "pthread_getattr_np: %s", strerror(rc));
		pthread_exit((void *)PTS_UNRESOLVED);
	}
	if ((rc = pthread_attr_getstack(&attr, &saddr, &ssize)) != 0) {
		printf(ERROR_PREFIX "pthread_attr_getstack: %s", strerror(rc));
		pthread_exit((void *)PTS_UNRESOLVED);
	}
	if (ssize != stack_size || saddr != stack_addr) {
		printf(ERROR_PREFIX "got the wrong stacksize or stackaddr");
		pthread_exit((void *)PTS_FAIL);
	}

	pthread_exit(NULL);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	pthread_attr_t attr;
	size_t ssize;
	void *saddr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init: %s", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	/* Get the default stack_addr and stack_size value */
	rc = pthread_attr_getstack(&attr, &stack_addr, &stack_size);
	if (rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_getstack: %s", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	/*
	 * Use smallest usable stack size for us to be able to call
	 * printf(3) / pthread_exit(3) without segfaulting.
	 *
	 * If for whatever reason PTHREAD_STACK_MIN is set to 0 (which it can
	 * be according to POSIX), posix_memalign will fail with EINVAL.
	 */
	stack_size = PTHREAD_STACK_MIN * 4;

	if ((rc = posix_memalign(&stack_addr, sysconf(_SC_PAGE_SIZE),
				 stack_size)) != 0) {
		printf(ERROR_PREFIX "posix_memalign: %s\n", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	if ((rc = pthread_attr_setstack(&attr, stack_addr, stack_size)) != 0) {
		printf(ERROR_PREFIX "pthread_attr_setstack: %s\n",
		       strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	if ((rc = pthread_attr_getstack(&attr, &saddr, &ssize)) != 0) {
		printf(ERROR_PREFIX "pthread_attr_getstack: %s\n",
		       strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	if ((rc = pthread_create(&new_th, &attr, thread_func, NULL)) != 0) {
		printf(ERROR_PREFIX "pthread_create: %s\n", strerror(rc));
		exit(PTS_FAIL);
	}

	if ((rc = pthread_join(new_th, NULL)) != 0) {
		printf(ERROR_PREFIX "pthread_join: %s\n", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	if ((rc = pthread_attr_destroy(&attr)) != 0) {
		printf(ERROR_PREFIX "pthread_attr_destroy: %s\n", strerror(rc));
		exit(PTS_UNRESOLVED);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
