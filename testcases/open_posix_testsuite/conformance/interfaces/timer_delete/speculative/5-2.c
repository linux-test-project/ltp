/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test to see if timer_delete() returns -1 and sets errno==EINVAL when
 * trying to delete a timer that has already been deleted or not.
 * Since this is a "may" assertion, either option is a pass.
 * Steps:
 * - Run test case 1-1.c and then try to delete the timer again.
 *
 * For this test, signal SIGTOTEST will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define TIMERSEC 3

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Should not have caught signal\n");
	exit(PTS_FAIL);
}

int main(void)
{
	struct sigevent ev;
	struct sigaction act;
	timer_t tid;
	struct itimerspec its;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	act.sa_handler = handler;
	act.sa_flags = 0;

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGTOTEST, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &its, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (sleep(TIMERSEC) != 0) {
		printf("sleep() did not sleep full time\n");
		return PTS_UNRESOLVED;
	}

	if (timer_delete(tid) == -1) {
		if (errno == EINVAL) {
			printf("fcn returned -1 and set errno=EINVAL\n");
			return PTS_PASS;
		} else {
			printf("errno!=EINVAL, but fcn returned -1\n");
			return PTS_FAIL;
		}
	}

	printf("fcn did not return -1\n");
	return PTS_PASS;
}
