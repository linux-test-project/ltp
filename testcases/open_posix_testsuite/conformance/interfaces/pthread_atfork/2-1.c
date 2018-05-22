/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * int pthread_atfork(void (*prepare) (void), void (*parent) (void), void (*child) (void))
 *
 * If no handling is desired at one or more of these three points, the corresponding fork
 * handler address(es) may be set to NULL.
 *
 * STEPS:
 * 1. Call pthread_atfork() with all NULL parameters
 * 2. Check to make sure the function returns success
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

int main(void)
{
	pid_t pid;
	int ret;

	/* Set up the fork handlers */
	ret = pthread_atfork(NULL, NULL, NULL);
	if (ret != 0) {
		if (ret == ENOMEM) {
			printf("Error: ran out of memory\n");
			return PTS_UNRESOLVED;
		}

		printf
		    ("Test FAILED: Expected return value success, instead received %d\n",
		     ret);
		return PTS_FAIL;
	}

	/* Now call fork() to make sure everything goes smoothly */
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

	printf("Test PASSED\n");
	return PTS_PASS;
}
