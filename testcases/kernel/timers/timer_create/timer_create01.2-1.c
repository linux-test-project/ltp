/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_create() returns a unique timer ID for the current
 * process.
 * For this test, signal SIGALRM will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <search.h>
#include <limits.h>
#include "posixtest.h"
#include "timer_create01.h"


int compare(const void *key, const void *amemb)
{
	if (*(timer_t *)key == *(timer_t *)amemb) {
		return 0;
	} else {
		return 1;
	}
}

int main(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	timer_t tids[TIMER_MAX];
	size_t i;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGALRM;

#ifdef DEBUG
	printf("Max timers is %ld\n", (long) TIMER_MAX);
#endif

	for (i=0; i<TIMER_MAX;i++) {
		if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
			perror("timer_create() did not return success\n");
			return PTS_UNRESOLVED;
		}

		tids[i] = tid;
		if (lfind(&tid, tids, &i, sizeof(timer_t), compare) != NULL) {
			printf("Duplicate tid found %d\n", tid);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("No duplicate tids found\n");
	printf("Test PASSED\n");
	return PTS_PASS;
}
