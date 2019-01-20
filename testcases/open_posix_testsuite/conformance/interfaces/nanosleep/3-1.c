/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() does not effect the action of a signal.
 * - Start nanosleep() in a child process
 * - Send a signal to the child process and have the handler exit with
 *   success; if the handler is not called, return 0
 * - In the parent, if the child exited success, return success.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "posixtest.h"

#define CHILDSUCCESS 1
#define CHILDFAIL 0

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Received signal - exit success\n");
	exit(CHILDSUCCESS);
}

int main(void)
{
	struct timespec tssleepfor, tsstorage;
	int sleepsec = 30;
	int pid;
	struct sigaction act;

	if ((pid = fork()) == 0) {
		/* child here */

		act.sa_handler = handler;
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) == -1) {
			perror("Error calling sigemptyset\n");
			return CHILDFAIL;
		}
		if (sigaction(SIGABRT, &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return CHILDFAIL;
		}
		tssleepfor.tv_sec = sleepsec;
		tssleepfor.tv_nsec = 0;
		nanosleep(&tssleepfor, &tsstorage);
		return CHILDFAIL;
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
			printf("Child exited normally\n");
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Child did not exit normally.\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	return PTS_UNRESOLVED;
}
