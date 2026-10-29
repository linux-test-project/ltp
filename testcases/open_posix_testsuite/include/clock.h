/*
 * Copyright (c) 2026, Linux Test Project
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PTS_CLOCK_H
#define PTS_CLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static inline int pts_mono_available(void)
{
#ifdef _POSIX_MONOTONIC_CLOCK
	if (_POSIX_MONOTONIC_CLOCK > 0)
		return 1;

	if (!_POSIX_MONOTONIC_CLOCK && sysconf(_SC_MONOTONIC_CLOCK) > 0)
		return 1;
#endif
	return 0;
}

static inline clockid_t pts_get_clock(void)
{
	if (pts_mono_available())
		return CLOCK_MONOTONIC;

	printf("CLOCK_MONOTONIC unavailable, test may fail due to clock adjustment\n");
	return CLOCK_REALTIME;
}

#define PTS_MONO_MAX_RETRIES 3

static struct timespec pts_mono_start;

static inline int pts_mono_time_start(void)
{
	if (!pts_mono_available()) {
		static int warned;

		if (!warned) {
			printf("CLOCK_MONOTONIC unavailable, test may fail due to clock adjustment\n");
			warned = 1;
		}
		return 0;
	}

#ifdef CLOCK_MONOTONIC
	if (clock_gettime(CLOCK_MONOTONIC, &pts_mono_start) != 0) {
		perror("clock_gettime(CLOCK_MONOTONIC) failed");
		return -1;
	}
#endif
	return 0;
}

static inline int pts_mono_time_check(unsigned int expected_secs)
{
#ifdef CLOCK_MONOTONIC
	if (pts_mono_available()) {
		struct timespec now;
		long elapsed;

		if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
			perror("clock_gettime(CLOCK_MONOTONIC) failed");
			return -1;
		}

		elapsed = now.tv_sec - pts_mono_start.tv_sec;

		if (labs(elapsed - (long)expected_secs) > 1) {
			printf("Clock adjustment detected (elapsed %lds, expected ~%us)\n",
			       elapsed, expected_secs);
			return 1;
		}
		return 0;
	}
#endif
	(void)expected_secs;
	return 0;
}

#endif /* PTS_CLOCK_H */
