/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Regression test motivated by an LKML discussion.  Test that nanosleep()
 * can be interrupted and then continue.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "posixtest.h"

#define SLEEPSEC 5

#define CHILDPASS 0		//if interrupted, child will return 0
#define CHILDFAIL 1

int main(void)
{
	int pid, slepts;
	struct timespec tsbefore, tsafter;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		struct timespec tssleep;

		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;
		if (nanosleep(&tssleep, NULL) == 0) {
			printf("nanosleep() returned success\n");
			return CHILDPASS;
		} else {
			printf("nanosleep() did not return success\n");
			return CHILDFAIL;
		}
		return CHILDFAIL;
	} else {
		/* parent here */
		int i;

		sleep(1);

		if (kill(pid, SIGSTOP) != 0) {
			printf("Could not raise SIGSTOP\n");
			return PTS_UNRESOLVED;
		}

		if (kill(pid, SIGCONT) != 0) {
			printf("Could not raise SIGCONT\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		if (!WIFEXITED(i)) {
			printf("nanosleep() did not return 0\n");
			return PTS_FAIL;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
			perror("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}

		slepts = tsafter.tv_sec - tsbefore.tv_sec;

		printf("Start %d sec; End %d sec\n", (int)tsbefore.tv_sec,
		       (int)tsafter.tv_sec);
		if (slepts >= SLEEPSEC) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("nanosleep() did not sleep long enough\n");
			return PTS_FAIL;
		}

	}			//end fork

	return PTS_UNRESOLVED;
}
