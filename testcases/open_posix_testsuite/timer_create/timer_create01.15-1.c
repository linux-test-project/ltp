/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_create() sets errno to EAGAIN if the system doesn't
 * have enough queuing resources to honor this request or the creation
 * of the timer would exceed the allowable timers that the calling process
 * can create.
 *
 * To do this, we will attempt to create a large number of timers.  
 * On some implementations, these results will be inconclusive.
 * 
 * For this test, signal SIGALRM will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define NUMTESTS 10000

int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	int i;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGALRM;

	for (i=0; i<NUMTESTS;i++) {
		if (timer_create(CLOCK_REALTIME, &ev, &tid) == -1) {
			printf("Stopped after %d timers.\n", i);
			if (EAGAIN == errno) {
				printf("Test PASSED\n");
				return PTS_PASS;
			} else {
				printf("errno != EAGAIN\n");
				printf("Test FAILED\n");
				return PTS_FAIL;
			}
		}

	}

	printf("All %d timers created!\n", NUMTESTS);
	printf("Test inconclusive\n");
	return PTS_UNRESOLVED;
}
