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

int caught_signal;

void handler(int signo)
{
	printf("Caught signal\n");
	caught_signal = 1;
}

int main(int argc, char *argv[])
{
#if _POSIX_CPUTIME == -1
	printf("_POSIX_CPUTIME not defined\n");
	return PTS_UNSUPPORTED;
#else
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsleft;
	time_t start_time, end_time;
	int overrun_time, rc;

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

	if (time(&start_time) == -1) {
		perror("time failed");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &ev, &tid) != 0) {
		perror("timer_create did not return success");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success");
		return PTS_UNRESOLVED;
	}

	if (nanosleep(&ts, &tsleft) != -1) {
		perror("nanosleep was not interrupted");
		return PTS_FAIL;
	}

	if ((end_time - start_time) == (TIMERSEC + overrun_amount)) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	printf("Timer did not last for correct amount of time\n"
		"timer: %d != correct %d\n",
		(int) (end_time - start_time), TIMERSEC);
	return PTS_FAIL;
#endif
}
