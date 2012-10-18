/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that the function:
 * int    sched_rr_get_interval(pid_t, struct timespec *);
 * is declared.
 */

#include <sched.h>
#include <time.h>
#include <sys/types.h>

typedef int (*sched_rr_get_interval_test) (pid_t, struct timespec *);

int dummyfcn(void)
{
	sched_rr_get_interval_test dummyvar;
	dummyvar = sched_rr_get_interval;
	return 0;
}
