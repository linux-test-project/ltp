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
 * Test if sched_getparam() sets errno == EFAULT or EINVAL if param points to
 * NULL.
 *
 * This behavior is not specified in the specs, so this test is speculative.
 */

#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	int result = -1;

	result = sched_getparam(0, NULL);

	if (result == -1 && errno == EFAULT) {
		printf("sched_getparam() sets errno == EFAULT "
		       "when param argument points to NULL\n");
		return PTS_PASS;
	}
	if (result == -1 && errno == EINVAL) {
		printf("sched_getparam() sets errno == EINVAL "
		       "when param argument points to NULL\n");
		return PTS_PASS;
	}

	printf("sched_getparam() return %i and sets errno == %i.\n",
	       result, errno);
	return PTS_UNRESOLVED;
}
