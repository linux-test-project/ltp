/*
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Here comes common funcions to correctly compute difference between two
 * struct timespec values.
 */

#define NSEC_IN_SEC 1000000000

#ifndef TIME_T_MAX
#	define TIME_T_MAX	(time_t)((1UL << ((sizeof(time_t) << 3) - 1)) - 1)
#endif

/*
 * Returns difference between two struct timespec values. If difference is
 * greater that 1 sec, 1 sec is returned.
 */
static inline long timespec_nsec_diff(struct timespec *t1, struct timespec *t2)
{
	time_t sec_diff;
	long nsec_diff;

	if (t2->tv_sec > t1->tv_sec) {
		struct timespec *tmp;
		tmp = t1;
		t1  = t2;
		t2  = tmp;
	}

	sec_diff  = t1->tv_sec - t2->tv_sec;
	nsec_diff = t1->tv_nsec - t2->tv_nsec;

	return labs(nsec_diff + NSEC_IN_SEC * sec_diff);
}
