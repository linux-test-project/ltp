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
 * int    sched_setparam(pid_t, const struct sched_param *);
 * is declared.
 */

#include <sched.h>
#include <sys/types.h>

typedef int (*sched_setparam_test) (pid_t, const struct sched_param *);

int dummyfcn(void)
{
	sched_setparam_test dummyvar;
	dummyvar = sched_setparam;
	return 0;
}
