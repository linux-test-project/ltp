/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_gettime() sets sets itimerspec.it_value to the
 * amount of time remaining in the middle of a timer.
 * - Create and arm a timer for TIMERNSEC nanoseconds.
 * - Sleep for SLEEPNSEC < TIMERNSEC.
 * - Call timer_gettime().
 * - Ensure the value returned is within ACCEPTABLEDELTA less than
 *   TIMERNSEC-SLEEPNSEC.
 *
 * Signal SIGCONT will be used so that it will not affect the test if
 * the timer expires.
 * Clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMERNSEC 80000000
#define SLEEPNSEC 40000000
#define ACCEPTABLEDELTA 3000000

int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec itsset, itsget;
	struct timespec ts;
	int deltans;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	itsset.it_interval.tv_sec = 0;
	itsset.it_interval.tv_nsec = 0;
	itsset.it_value.tv_sec = 0;
	itsset.it_value.tv_nsec = TIMERNSEC;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &itsset, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec=0;
	ts.tv_nsec=SLEEPNSEC;
	if (nanosleep(&ts, NULL) != 0) {
		perror("nanosleep() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_gettime(tid, &itsget) != 0) {
		perror("timer_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * Algorithm for determining success:
	 * - itsget must be < itsset
	 * - itsset-itsget nsec must be <= ACCEPTABLEDELTA
	 */

	if (itsget.it_value.tv_sec > 0) {
		printf("FAIL:  timer_gettime() value > time expected left\n");
		printf("%d seconds > 0 seconds\n", 
				(int) itsget.it_value.tv_sec);
		return PTS_FAIL;
	}

	deltans=(itsset.it_value.tv_nsec - ts.tv_nsec)- itsget.it_value.tv_nsec;

	if (deltans < 0) {
		printf("FAIL:  timer_gettime() value > time expected left\n");
		printf("%d > %d\n", (int) itsget.it_value.tv_nsec, 
				(int) itsset.it_value.tv_nsec - 
					(int) ts.tv_nsec);
		return PTS_FAIL;
	}

	if (deltans <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("FAIL:  timer_gettime() value !~= time expected left\n");
		printf("%d !~= %d\n", (int) itsget.it_value.tv_nsec, 
				(int) itsset.it_value.tv_nsec - 
					(int) ts.tv_nsec);
		return PTS_FAIL;
	}

	printf("This code should not be executed\n");
	return PTS_UNRESOLVED;
}
