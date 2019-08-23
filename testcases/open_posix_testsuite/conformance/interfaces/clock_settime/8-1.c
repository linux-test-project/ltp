/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that clock_settime() does not affect clock_nanosleep()
 * relative timers expiration times.
 *
 * Steps:
 * - get time T0
 * - in child:  set clock_nanosleep() to sleep for SLEEPSEC seconds
 * - in parent:  sleep SMALLTIME (< SLEEPSEC)
 * - in parent:  set time back to T0
 * - in child:  ensure time when clock_nanosleep() expires is within
 *   ACCEPTABLEDELTA of T0+(SLEEPSEC-SMALLTIME)
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"
#include "helpers.h"

#define SLEEPSEC 5
#define SMALLTIME 2
#define ACCEPTABLEDELTA 1

#define CHILDPASS 1
#define CHILDFAIL 0

int main(void)
{
	struct timespec tsT0, tssleep;
	int pid;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (getuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}

	if (clock_gettime(CLOCK_REALTIME, &tsT0) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		struct timespec tsend;
		time_t expectedsec;

		tssleep.tv_sec = SLEEPSEC;
		tssleep.tv_nsec = 0;

		if (clock_nanosleep(CLOCK_REALTIME, 0, &tssleep, NULL) != 0) {
			printf("clock_nanosleep() did not return success\n");
			return CHILDFAIL;
		}

		if (clock_gettime(CLOCK_REALTIME, &tsend) != 0) {
			perror("clock_gettime() did not return success\n");
			return CHILDFAIL;
		}

		expectedsec = tsT0.tv_sec + (SLEEPSEC - SMALLTIME);

		if (tsend.tv_sec >= expectedsec) {
			if ((tsend.tv_sec - expectedsec) <= ACCEPTABLEDELTA) {
				return CHILDPASS;
			} else {
				printf("Ended too late.  %d >> %d\n",
				       (int)tsend.tv_sec, (int)expectedsec);
				return CHILDFAIL;
			}
		} else {
			printf("Did not sleep for long enough %d < %d\n",
			       (int)tsend.tv_sec, (int)expectedsec);
			return CHILDFAIL;
		}

		return CHILDFAIL;
	} else {
		/* parent here */
		int i;
		struct timespec tsreset;

		sleep(SMALLTIME);

		if (clock_settime(CLOCK_REALTIME, &tsT0) != 0) {
			printf("clock_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		getBeforeTime(&tsreset);	// get current time
		tsreset.tv_sec += SMALLTIME;
		setBackTime(tsreset);

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
