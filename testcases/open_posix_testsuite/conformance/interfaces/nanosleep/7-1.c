/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() sets errno to EINTR if it is interrupted by a signal.
 * Test by sending a signal to a child doing nanosleep().  If nanosleep
 * errno = EINTR, return success from the child.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "posixtest.h"

#define CHILDSUCCESS 1
#define CHILDFAILURE 0

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("In handler\n");
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
			return CHILDFAILURE;
		}
		if (sigaction(SIGABRT, &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return CHILDFAILURE;
		}
		tssleepfor.tv_sec = sleepsec;
		tssleepfor.tv_nsec = 0;
		if (nanosleep(&tssleepfor, &tsstorage) == -1) {
			if (EINTR == errno) {
				printf("errno == EINTR\n");
				return CHILDSUCCESS;
			} else {
				printf("errno != EINTR\n");
				return CHILDFAILURE;
			}
		} else {
			printf("nanosleep did not return -1\n");
			return CHILDFAILURE;
		}

		return CHILDFAILURE;
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
			printf("Child did not exit normally.\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	return PTS_UNRESOLVED;
}
