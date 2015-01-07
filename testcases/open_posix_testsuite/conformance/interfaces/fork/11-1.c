/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
* Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

* This sample test aims to check the following assertion:
*
* The file locks are not inherited by the child process.

* The steps are:
* -> lock stdout
* -> fork
* -> child creates a thread
* -> child thread trylock stdout
* -> join the child

* The test fails if the child thread cannot lock the file
* -- this would mean the child process got stdout file lock ownership.

*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <errno.h>

#include "posixtest.h"

static void *threaded(void *arg)
{
	int ret;
	long res;

	(void) arg;

	ret = ftrylockfile(stdout);

	if (ret) {
		res = PTS_FAIL;
		printf("FAIL: The child process inherited the lock\n");
	} else {
		res = PTS_PASS;
	}

	funlockfile(stdout);

	return (void*)res;
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;
	pthread_t ch;
	long res;

	/* lock the stdout file */
	flockfile(stdout);

	child = fork();

	if (child == -1) {
		funlockfile(stdout);
		perror("fork");
		return PTS_UNRESOLVED;
	}

	if (child == 0) {
		/* Setup timeout in case the thread hangs in the lock */
		alarm(1);

		/*
		 * We have to try to acquire the lock from different thread
		 * because the file locks are recursive.
		 */
		ret = pthread_create(&ch, NULL, threaded, NULL);

		if (ret != 0) {
			printf("pthread_create: %s\n", strerror(ret));
			exit(PTS_UNRESOLVED);
		}

		ret = pthread_join(ch, (void*)&res);

		if (ret != 0) {
			printf("pthread_join: %s\n", strerror(ret));
			exit(PTS_UNRESOLVED);
		}

		exit(res);
	}

	funlockfile(stdout);

	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		printf("Waitpid returned the wrong PID\n");
		return PTS_UNRESOLVED;
	}

	if (!WIFEXITED(status)) {
		printf("FAIL: Child exited abnormaly, timeout?\n");
		return PTS_FAIL;
	}

	if (WEXITSTATUS(status) != PTS_PASS)
		return WEXITSTATUS(status);

	printf("Test PASSED\n");
	return PTS_PASS;
}
