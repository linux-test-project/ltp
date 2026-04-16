/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that if clock_settime() changes the time for CLOCK_REALTIME,
 * then any threads blocked on clock_nanosleep() for the CLOCK_REALTIME
 * clock that would now have expired in the past will expire immediately.
 *
 * Steps:
 * - get time T0
 * - in child:  set clock_nanosleep() to sleep until time
 *              T1 = T0 + SLEEPOFFSET
 * - in parent:  set time forward to T2 = T1 + SMALLTIME
 * - in child:  ensure clock_nanosleep() expires within ACCEPTABLEDELTA of
 *              T2
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "posixtest.h"
#include "helpers.h"

#define SLEEPOFFSET 5
#define SMALLTIME 2
#define ACCEPTABLEDELTA 1

#define CHILDPASS 1
#define CHILDFAIL 0
#define CHILDUNTESTED 2
#define CHILDUNRESOLVED 3

int test_main(int argc PTS_ATTRIBUTE_UNUSED, char **argv PTS_ATTRIBUTE_UNUSED)
{
	struct timespec tsT0, tsT1, tsT2, tsreset;
	int pid, child_status;
	int attempt, ret;

	/* Check that we're root...can't call clock_settime with CLOCK_REALTIME otherwise */
	if (getuid() != 0) {
		printf("Run this test as ROOT, not as a Regular User\n");
		return PTS_UNTESTED;
	}

	for (attempt = 0; attempt < PTS_MONO_MAX_RETRIES; attempt++) {
		if (clock_gettime(CLOCK_REALTIME, &tsT0) != 0) {
			perror("clock_gettime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		tsT1.tv_sec = tsT0.tv_sec + SLEEPOFFSET;
		tsT1.tv_nsec = tsT0.tv_nsec;

		tsT2.tv_sec = tsT1.tv_sec + SMALLTIME;
		tsT2.tv_nsec = tsT1.tv_nsec;

		pid = fork();
		if (pid < 0) {
			perror("fork() failed");
			return PTS_UNRESOLVED;
		}
		if (pid == 0) {
			/* child here */
			int flags = 0;
			struct timespec tsT3;

			flags |= TIMER_ABSTIME;

			if (pts_mono_time_start() != 0)
				return CHILDUNRESOLVED;

			if (clock_nanosleep(CLOCK_REALTIME, flags, &tsT1, NULL) != 0) {
				printf("clock_nanosleep() did not return success\n");
				return CHILDFAIL;
			}

			/*
			 * The parent sleeps 1s before jumping the clock forward.
			 * If clock_nanosleep returned too quickly, an external
			 * clock adjustment (NTP, VM sync) woke us instead of the
			 * parent's clock_settime.
			 */
			ret = pts_mono_time_check(1);
			if (ret < 0)
				return CHILDUNRESOLVED;
			if (ret > 0)
				return CHILDUNTESTED;

			if (clock_gettime(CLOCK_REALTIME, &tsT3) != 0) {
				perror("clock_gettime() did not return success\n");
				return CHILDFAIL;
			}

			if (tsT3.tv_sec >= tsT2.tv_sec) {
				if ((tsT3.tv_sec - tsT2.tv_sec) <= ACCEPTABLEDELTA)
					return CHILDPASS;

				printf("Ended too late.  %d >> %d\n",
				       (int)tsT3.tv_sec, (int)tsT2.tv_sec);
				return CHILDFAIL;
			}

			printf("Did not sleep for long enough %d < %d\n",
			       (int)tsT3.tv_sec, (int)tsT2.tv_sec);
			return CHILDFAIL;
		}

		/* parent here */
		sleep(1);

		getBeforeTime(&tsreset);
		if (clock_settime(CLOCK_REALTIME, &tsT2) != 0) {
			printf("clock_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (wait(&child_status) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		setBackTime(tsreset);

		if (!WIFEXITED(child_status))
			break;

		if (WEXITSTATUS(child_status) == CHILDUNRESOLVED)
			return PTS_UNRESOLVED;

		if (WEXITSTATUS(child_status) != CHILDUNTESTED)
			break;
	}

	if (attempt == PTS_MONO_MAX_RETRIES) {
		printf("UNTESTED: persistent clock interference after %d attempts\n",
		       PTS_MONO_MAX_RETRIES);
		return PTS_UNTESTED;
	}

	if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == CHILDPASS) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED\n");
	return PTS_FAIL;
}
