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

#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <search.h>
#include <limits.h>
#include "posixtest.h"

int compare(const void *key, const void *amemb)
{
	if (*(timer_t *) key == *(timer_t *) amemb) {
		return 0;
	} else {
		return 1;
	}
}

int main(void)
{
	struct sigevent ev;
	timer_t tid;
	timer_t *tids;
	size_t i;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGALRM;

#if defined DEBUG && defined TIMER_MAX
	printf("Max timers is %ld\n", (long)TIMER_MAX);
	int max = TIMER_MAX;
#else
	int max = 256;
#endif
	tids = malloc(max * sizeof(timer_t));
	if (tids == NULL) {
		perror("malloc failed\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; (int)i < max; i++) {
		if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
#ifndef TIMER_MAX
			if (errno == EAGAIN)
				break;
#endif
			perror("timer_create() did not return success\n");
			return PTS_UNRESOLVED;
		}

		tids[i] = tid;
		if (lfind(&tid, tids, &i, sizeof(timer_t), compare) != NULL) {
			printf("Duplicate tid found %ld\n", (long)tid);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("No duplicate tids found\n");
	printf("Test PASSED\n");
	return PTS_PASS;
}
