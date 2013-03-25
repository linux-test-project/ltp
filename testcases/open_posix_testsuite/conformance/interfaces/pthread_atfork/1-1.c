/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * int pthread_atfork(void (*prepare) (void), void (*parent) (void), void (*child) (void))
 *
 *  shall declare fork handlers to be called before and after a fork() command, in the context
 *  of the thread that called the fork().  The 'prepare' fork handler shall be called before
 *  fork() processing commences.  The 'parent' fork handle shall be called after fork()
 *  processing completes in the parent process.  The 'child' fork shall be called after
 *  fork() processing completes in the child process.
 *
 * STEPS:
 * 1. Call pthread_atfork() before a fork() call, setting all three fork handlers for
 *    prepare, parent and child.
 * 2. Call fork()
 * 3. Verify that all three fork handlers were called
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "posixtest.h"

#define HANDLER_NOTCALLED 0
#define HANDLER_CALLED 1

int prep_val;
int parent_val;
int child_val;

static void prepare_handler()
{
	prep_val = HANDLER_CALLED;
	return;
}

static void parent_handler()
{
	parent_val = HANDLER_CALLED;
	return;
}

static void child_handler()
{
	child_val = HANDLER_CALLED;
	return;
}

int main(void)
{
	pid_t pid;

	/* Initialize values */
	prep_val = HANDLER_NOTCALLED;
	parent_val = HANDLER_NOTCALLED;
	child_val = HANDLER_NOTCALLED;

	/* Set up the fork handlers */
	if (pthread_atfork(prepare_handler, parent_handler, child_handler) != 0) {
		printf("Error in pthread_atfork\n");
		return PTS_UNRESOLVED;
	}

	/* Now call fork() */
	pid = fork();

	if (pid < 0) {
		perror("Error in fork()\n");
		return PTS_UNRESOLVED;
	}
	if (pid == 0) {
		/* Child process */
		pthread_exit(0);
	} else {
		/* Parent process */
		wait(NULL);
	}

	/* Check if fork handlers were called */
	if (prep_val == 1) {
		if (parent_val == 1) {
			if (parent_val == 1) {
				printf("Test PASSED\n");
				return PTS_PASS;
			} else {
				printf
				    ("Test FAILED: child handler not called\n");
				return PTS_FAIL;
			}
		} else {
			printf("Test FAILED: parent handler not called\n");
			return PTS_FAIL;
		}
	} else {
		printf("Test FAILED: prepare handler not called\n");
		return PTS_FAIL;
	}

	/* Should not reach here */
	printf("Error: control should not reach here\n");
	return PTS_UNRESOLVED;
}
