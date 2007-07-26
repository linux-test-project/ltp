/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 * adam li
 *
 * Test that CLOCK_THREAD_CPUTIME_ID is supported by timer_create().
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
#define SLEEPDELTA 3
#define ACCEPTABLEDELTA 1

void handler(int signo)
{
	printf("Caught signal\n");
}

int main(int argc, char *argv[])
{
	int rc;
	rc = sysconf(_SC_THREAD_CPUTIME);
	printf("rc = %d\n", rc);

#if _POSIX_THREAD_CPUTIME != -1
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;
	struct timespec ts, tsleft;

	if (rc == -1) {
		printf("_POSIX_THREAD_CPUTIME unsupported\n");
		return PTS_UNSUPPORTED;
	}

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler=handler;
	act.sa_flags=0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	ts.tv_sec=TIMERSEC+SLEEPDELTA;
	ts.tv_nsec=0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_THREAD_CPUTIME_ID, &ev, &tid) != 0) {
		perror("timer_create() did not return success");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success");
		return PTS_UNRESOLVED;
	}

	if (nanosleep(&ts, &tsleft) != -1) {
		perror("nanosleep() not interrupted");
		return PTS_FAIL;
	}

	if ( abs(tsleft.tv_sec-SLEEPDELTA) <= ACCEPTABLEDELTA) {
		printf("Test PASSED");
		return PTS_PASS;
	} else {
		printf("Timer did not last for correct amount of time\n");
		printf("timer: %d != correct %d\n", 
				(int) ts.tv_sec- (int) tsleft.tv_sec,
				TIMERSEC);
		return PTS_FAIL;
	}

	return PTS_UNRESOLVED;
#else
	printf("_POSIX_THREAD_CPUTIME unsupported\n");
	return PTS_UNSUPPORTED;
#endif

}
