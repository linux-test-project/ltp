/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam li
 *
 * Test that CLOCK_PROCESS_CPUTIME_ID is supported by timer_create().
 *
 * Same test as 1-1.c.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define TIMERSEC 2
#define ACCEPTABLEDELTA 1

static volatile int caught_signal;

static void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	caught_signal = 1;
}

int main(void)
{
#if _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME not defined\n");
	return PTS_UNSUPPORTED;
#else
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts_start, ts_end;
	int rc;

	rc = sysconf(_SC_CPUTIME);
	if (rc == -1) {
		printf("_SC_CPUTIME unsupported\n");
		return PTS_UNRESOLVED;
	}

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset");
		return PTS_UNRESOLVED;
	}

	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &ev, &tid) != 0) {
		perror("timer_create did not return success");
		return PTS_UNRESOLVED;
	}

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_start) != 0) {
		perror("clock_gettime() failed");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime did not return success");
		return PTS_UNRESOLVED;
	}

	/*
	 * The bussy loop is intentional. The signal is send after
	 * two seconds of CPU time has been accumulated.
	 */
	while (!caught_signal) ;

	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_end) != 0) {
		perror("clock_gettime() failed");
		return PTS_UNRESOLVED;
	}

	if (labs(ts_end.tv_sec - ts_start.tv_sec - TIMERSEC) <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Timer did not last for correct amount of time\n"
	       "stop - start = %d - %d > %d\n",
	       (int)ts_end.tv_sec, (int)ts_start.tv_sec,
	       TIMERSEC + ACCEPTABLEDELTA);
	return PTS_FAIL;
#endif
}
