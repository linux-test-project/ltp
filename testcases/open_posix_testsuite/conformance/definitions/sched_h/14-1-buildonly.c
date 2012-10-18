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
 * int    sched_getscheduler(pid_t);
 * is declared.
 */

#include <sched.h>
#include <sys/types.h>

typedef int (*sched_getscheduler_test) (pid_t);

int dummyfcn(void)
{
	sched_getscheduler_test dummyvar;
	dummyvar = sched_getscheduler;
	return 0;
}
