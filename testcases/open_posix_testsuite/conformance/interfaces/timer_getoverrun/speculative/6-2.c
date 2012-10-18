/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test to see if timer_getoverrun() returns -1 and sets errno==EINVAL when
 * trying to call timer_getoverrun on a timer that has been deleted or not.
 * Since this assertion is a "may," either option is a pass.
 *
 * For this test, signal SIGCONT will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define TIMERSEC 3

int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_delete(tid) != 0) {
		perror("timer_delete() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_getoverrun(tid) == -1) {
		if (errno==EINVAL) {
			printf("fcn returned -1 and set errno=EINVAL\n");
			return PTS_PASS;
		} else {
			printf("fcn returned -1 but errno!=EINVAL\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("fcn did not return -1\n");
	return PTS_PASS;
}
