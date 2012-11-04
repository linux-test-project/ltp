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
 * int    sched_getparam(pid_t, struct sched_param *);
 * is declared.
 */

#include <sched.h>
#include <sys/types.h>

typedef int (*sched_getparam_test) (pid_t, struct sched_param *);

int dummyfcn(void)
{
	sched_getparam_test dummyvar;
	dummyvar = sched_getparam;
	return 0;
}
