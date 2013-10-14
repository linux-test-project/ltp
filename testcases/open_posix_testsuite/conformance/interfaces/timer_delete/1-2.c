/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that after a timer is deleted by calling timer_delete(), it
 * cannot be armed by calling timer_settime().
 * Steps:
 * - Create a timer
 * - Delete that timer
 * - Try to call timer_settime() on that timer and ensure it fails
 *   with errno==EINVAL.
 *
 * For this test, signal SIGTOTEST will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define SIGTOTEST SIGALRM
#define TIMERSEC 3

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGTOTEST;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = TIMERSEC;
	its.it_value.tv_nsec = 0;

	if (timer_settime(tid, 0, &its, NULL) == -1) {
		if (errno == EINVAL) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno!=EINVAL after a timer_delete()\n");
			return PTS_FAIL;
		}
	}

	printf("timer_settime() did not fail after timer_delete()\n");
	return PTS_FAIL;
}
