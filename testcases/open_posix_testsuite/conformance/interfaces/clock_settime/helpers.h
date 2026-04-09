/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Helper functions used to reset the time to close to the current
 * time at the end of the test.
 *
 * Since these helper functions are made specifically to be included
 * in certain tests, they make use of some libraries already included
 * by those tests.
 */

#include <stdlib.h>

static int getBeforeTime(struct timespec *tpget)
{
	if (clock_gettime(CLOCK_REALTIME, tpget) != 0) {
		perror("clock_gettime() did not return success\n");
		perror("clock may not be reset properly\n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}

static int setBackTime(struct timespec tpset)
{
	if (clock_settime(CLOCK_REALTIME, &tpset) != 0) {
		perror("clock_settime() did not return success\n");
		perror("clock may not be reset properly\n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}

#define PTS_MONO_MAX_RETRIES 3

#ifdef _POSIX_MONOTONIC_CLOCK
static struct timespec _pts_mono_start;

static inline int pts_mono_time_start(void)
{
	if (clock_gettime(CLOCK_MONOTONIC, &_pts_mono_start) != 0) {
		perror("clock_gettime(CLOCK_MONOTONIC) failed");
		return -1;
	}
	return 0;
}

static inline int pts_mono_time_check(unsigned int expected_secs)
{
	struct timespec now;
	long elapsed;

	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
		perror("clock_gettime(CLOCK_MONOTONIC) failed");
		return -1;
	}

	elapsed = now.tv_sec - _pts_mono_start.tv_sec;

	if (labs(elapsed - (long)expected_secs) > 1) {
		printf("Clock adjustment detected (elapsed %lds, expected ~%us)\n",
		       elapsed, expected_secs);
		return 1;
	}
	return 0;
}
#else
static inline int pts_mono_time_start(void)
{
	static int warned;

	if (!warned) {
		printf("CLOCK_MONOTONIC unavailable, test may fail due to clock adjustment\n");
		warned = 1;
	}
	return 0;
}

static inline int pts_mono_time_check(unsigned int expected_secs)
{
	(void)expected_secs;
	return 0;
}
#endif

