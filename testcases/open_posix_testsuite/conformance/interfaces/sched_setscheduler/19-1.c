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
 * Test that sched_setscheduler() sets errno == EINVAL when the sched_priority
 * member is not within the inclusive priority range for the scheduling policy.
 *
 * Test is done for all policy defined in the spec into a loop.
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

struct unique {
	int value;
	char *name;
} sym[] = {

	{
	SCHED_FIFO, "SCHED_FIFO"}, {
	SCHED_RR, "SCHED_RR"},
#if defined(_POSIX_SPORADIC_SERVER)&&(_POSIX_SPORADIC_SERVER != -1) || defined(_POSIX_THREAD_SPORADIC_SERVER)&&(_POSIX_THREAD_SPORADIC_SERVER != -1)
	{
	SCHED_SPORADIC, "SCHED_SPORADIC"},
#endif
	{
	SCHED_OTHER, "SCHED_OTHER"}, {
	0, 0}
};

int main(void)
{
	int policy, invalid_priority, tmp, result = PTS_PASS;
	struct sched_param param;

	struct unique *tst;

	tst = sym;
	while (tst->name) {
		policy = tst->value;
		fflush(stderr);
		printf("Policy: %s\n", tst->name);
		fflush(stdout);

		invalid_priority = sched_get_priority_max(policy);
		if (invalid_priority == -1) {
			perror
			    ("An error occurs when calling sched_get_priority_max()");
			return PTS_UNRESOLVED;
		}

		/* set an invalid priority */
		invalid_priority++;
		param.sched_priority = invalid_priority;

		tmp = sched_setscheduler(0, policy, &param);

		if (tmp == -1 && errno == EINVAL) {
			printf("  OK\n");
		} else if (tmp != -1) {
			printf("  The returned code is not -1.\n");
			result = PTS_FAIL;
		} else if (errno == EPERM) {
			printf
			    ("  This process does not have the permission to set its own scheduling policy.\n  Try to launch this test as root.\n");
			if (result != PTS_FAIL) {
				result = PTS_UNRESOLVED;
			}
		} else {
			perror("  Unknow error");
			result = PTS_FAIL;
		}

		tst++;
	}

	if (result == PTS_PASS) {
		printf("Test PASSED\n");
	}
	return result;
}
