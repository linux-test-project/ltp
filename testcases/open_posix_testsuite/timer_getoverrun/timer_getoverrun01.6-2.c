/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_getoverrun() returns -1 and sets errno==EINVAL when
 * trying to call timer_getoverrun on a timer that has been deleted.
 *
 * For this test, signal SIGCONT will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
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
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("errno!=EINVAL when deleted timer ID sent\n");
			return PTS_FAIL;
		}
	} else {
		printf("timer_getoverrun() didn't fail on deleted timer ID\n");
		return PTS_FAIL;
	}

	printf("This code should never be executed\n");
	return PTS_UNRESOLVED;
}
