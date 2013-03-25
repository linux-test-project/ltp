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
 * Test that sched_setscheduler() sets the scheduling policy and scheduling
 * parameters of the process specified by pid to policy and the parameters
 * specified in the sched_param structure pointed to by param, respectively.
 *
 * For all policy describe in the spec, the test will check the policy and the
 * param of the process after the call of sched_setscheduler.
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
	int tmp, policy, priority, result = PTS_PASS;
	struct sched_param param;
	struct unique *tst;

	tst = sym;
	while (tst->name) {
		fflush(stderr);
		printf("Policy: %s\n", tst->name);
		fflush(stdout);

		policy = tst->value;
		priority = (sched_get_priority_min(policy) +
			    sched_get_priority_max(policy)) / 2;
		param.sched_priority = priority;

		tmp = sched_setscheduler(getpid(), policy, &param);

		if (tmp == -1 || errno != 0) {
			if (errno == EPERM) {
				printf
				    ("  The process do not have permission to change its own scheduler\n  Try to run this test as root.\n");
			} else {
				printf
				    ("  Error calling sched_setscheduler() for %s policy\n",
				     tst->name);
			}
			if (result != PTS_FAIL)
				result = PTS_UNRESOLVED;
			tst++;
			continue;
		}

		if (sched_getparam(getpid(), &param) != 0) {
			perror("Error calling sched_getparam()");
			return PTS_UNRESOLVED;
		}

		if (policy != sched_getscheduler(getpid())) {
			printf
			    ("  sched_setscheduler() does not set the policy to %s.\n",
			     tst->name);
			result = PTS_FAIL;
		}
		if (priority != param.sched_priority) {
			printf
			    ("  sched_setscheduler() does not set the right param for %s policy.\n",
			     tst->name);
			result = PTS_FAIL;
		}

		tst++;
	}

	if (result == PTS_PASS)
		printf("Test PASSED\n");
	return result;
}
