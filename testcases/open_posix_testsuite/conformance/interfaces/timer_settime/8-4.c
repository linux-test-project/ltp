/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_settime() will return ovalue.it_interval = the previous
 * reload value.
 * - set up a timer to expire in TIMERSEC seconds with reload value RELOADVAL
 * - set up a new timer
 * - ensure ovalue.it_interval = RELOADVAL
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

#define TIMERSEC 1
#define RELOADVAL 8

int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its, oits;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	its.it_interval.tv_sec = RELOADVAL;
	its.it_value.tv_sec = TIMERSEC;
	its.it_interval.tv_nsec = 0; its.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, &oits) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * second call to timer_settime()
	 */
	if (timer_settime(tid, 0, &its, &oits) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (RELOADVAL == oits.it_interval.tv_sec) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED:  correct %d oits.it_interval %d\n",
				RELOADVAL, (int) oits.it_interval.tv_sec);
		return PTS_FAIL;
	}

	printf("This code should not be executed.\n");
	return PTS_UNRESOLVED;
}
