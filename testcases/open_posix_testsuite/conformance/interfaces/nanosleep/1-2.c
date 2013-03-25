/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that nanosleep() causes the current thread to be suspended
 * until a signal whose action is to terminate the process is received.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"

int main(void)
{
	struct timespec tssleepfor, tsstorage, tsbefore, tsafter;
	int sleepsec = 30;
	int pid;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
		perror("Error in clock_gettime()\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		tssleepfor.tv_sec = sleepsec;
		tssleepfor.tv_nsec = 0;
		nanosleep(&tssleepfor, &tsstorage);
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
		if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
			perror("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}

		/*
		 * pass if we slept for less than the (large) sleep time
		 * allotted
		 */
		if ((tsafter.tv_sec - tsbefore.tv_sec) < sleepsec) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Slept for too long: %d >= %d\n",
			       (int)tsafter.tv_sec - (int)tsbefore.tv_sec,
			       sleepsec);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	return PTS_UNRESOLVED;
}
