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
 * int    sched_get_priority_max(int);
 * is declared.
 */

#include <sched.h>

typedef int (*sched_get_priority_max_test) (int);

int dummyfcn(void)
{
	sched_get_priority_max_test dummyvar;
	dummyvar = sched_get_priority_max;
	return 0;
}
