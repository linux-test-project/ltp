/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_settime() will return ovalue.it_value = the previous
 * amount of time before the timer would have expired.
 * - set up a timer to expire in TIMERSEC seconds
 * - sleep for TIMERSEC-TIMELEFT seconds
 * - set up a new timer
 * - compare TIMELEFT and the value in ovalue.it_value and ensure they
 *   are within ACCEPTABLEDELTA of each other
 *
 * For this test, signal SIGCONT will be used so that the test will
 * not abort.  Clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMERSEC 7
#define TIMELEFT 5
#define ACCEPTABLEDELTA 1

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its, oits;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, &oits) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	sleep(TIMERSEC - TIMELEFT);

	if (timer_settime(tid, 0, &its, &oits) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (labs(oits.it_value.tv_sec - TIMELEFT) <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("Test FAILED:  time left %d oits.it_value.tv_sec %d\n",
	       TIMELEFT, (int)oits.it_value.tv_sec);
	return PTS_FAIL;
}
