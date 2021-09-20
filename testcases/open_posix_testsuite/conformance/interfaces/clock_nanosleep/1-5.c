/*
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that clock_nanosleep() does not stop if a signal is received
 * that has no signal handler.  clock_nanosleep() should still respond
 * to the signal, but should resume after a SIGCONT signal is received.
 *
 * SIGSTOP will be used to stop the sleep.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "posixtest.h"
#include "proc.h"

#define SLEEPSEC 5

#define CHILDPASS 1
#define CHILDFAIL 0

int main(void)
{
	int pid, slepts;
	struct timespec tsbefore, tsafter;

	if (clock_gettime(CLOCK_REALTIME, &tsbefore) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) < 0) {
		printf("fork() did not return success\n");
		return PTS_UNRESOLVED;
	} else if (pid == 0) {
		/* child here */
		struct timespec tssleep;

		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;
		if (clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL) == 0) {
			printf("clock_nanosleep() returned success\n");
			return CHILDPASS;
		} else {
			printf("clock_nanosleep() did not return success\n");
			return CHILDFAIL;
		}
		return CHILDFAIL;
	} else {
		/* parent here */
		int i;

		tst_process_state_wait3(pid, 'S', 1);

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

		if (!WIFEXITED(i) || !WEXITSTATUS(i)) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
			perror("Error in clock_gettime()\n");
			return PTS_UNRESOLVED;
		}

		slepts = tsafter.tv_sec - tsbefore.tv_sec;

#ifdef DEBUG
		printf("Start %d sec; End %d sec\n", (int)tsbefore.tv_sec,
		       (int)tsafter.tv_sec);
#endif
		if (slepts >= SLEEPSEC) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("clock_nanosleep() did not sleep long enough\n");
			return PTS_FAIL;
		}

	}			//end fork

	return PTS_UNRESOLVED;
}
