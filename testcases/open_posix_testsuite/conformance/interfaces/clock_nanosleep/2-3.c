/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() causes the current thread to be suspended
 * until a signal whose action is to terminate the process is received
 * for an absolute clock.  [Same as test 1-4.c except with TIMER_ABSTIME
 * set.]
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
	time_t sleepuntilsec;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleepuntilsec = tsbefore.tv_sec + SLEEPSEC;

	if ((pid = fork()) == 0) {
		int flags = 0;

		/* child here */
		tssleep.tv_sec = sleepuntilsec;
		tssleep.tv_nsec = tsbefore.tv_nsec;

		flags |= TIMER_ABSTIME;
		clock_nanosleep(CLOCK_REALTIME, flags, &tssleep, NULL);
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
		if (tsafter.tv_sec < sleepuntilsec) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Slept for too long: %lld >= %lld\n",
			       (long long int)tsafter.tv_sec,
				   (long long int)sleepuntilsec);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	return PTS_UNRESOLVED;
}
