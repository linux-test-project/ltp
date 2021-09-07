/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() causes the current thread to be suspended
 * until a signal whose action is to terminate the process is received.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"
#include "proc.h"

#define SLEEPSEC 30

int main(void)
{
	struct timespec tssleep, tsbefore, tsafter;
	int pid;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;
		clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL);
	} else {
		/* parent here */
		int i;

		tst_process_state_wait3(pid, 'S', 1);

		if (kill(pid, SIGABRT) != 0) {
			printf("Could not raise signal being tested\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}
		if (clock_gettime(CLOCK_REALTIME, &tsafter) != 0) {
			perror("clock_gettime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		/*
		 * pass if we slept for less than the (large) sleep time
		 * allotted
		 */
		if ((tsafter.tv_sec - tsbefore.tv_sec) < SLEEPSEC) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Slept for too long: %d >= %d\n",
			       (int)tsafter.tv_sec - (int)tsbefore.tv_sec,
			       SLEEPSEC);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	return PTS_UNRESOLVED;
}
