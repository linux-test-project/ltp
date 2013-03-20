/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() sets errno=EINTR if it was interruped
 * by a signal.  Test for relative sleep.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "posixtest.h"

#define SLEEPSEC 30

#define CHILDPASS 1
#define CHILDFAIL 0

void handler(int signo)
{
	(void) signo;

	printf("In handler\n");
}

int main(void)
{
	struct timespec tssleep;
	int pid;
	struct sigaction act;

	if ((pid = fork()) == 0) {
		/* child here */

		act.sa_handler = handler;
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) != 0) {
			perror("sigemptyset() did not return success\n");
			return CHILDFAIL;
		}
		if (sigaction(SIGABRT, &act, 0) != 0) {
			perror("sigaction() did not return success\n");
			return CHILDFAIL;
		}
		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;
		if (clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL) == EINTR) {
			return CHILDPASS;
		} else {
			printf("errno != EINTR\n");
			return CHILDFAIL;
		}
	} else {
		/* parent here */
		int i;

		sleep(1);

		if (kill(pid, SIGABRT) != 0) {
			printf("Could not raise signal being tested\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		if (WIFEXITED(i) && WEXITSTATUS(i)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	return PTS_UNRESOLVED;
}
