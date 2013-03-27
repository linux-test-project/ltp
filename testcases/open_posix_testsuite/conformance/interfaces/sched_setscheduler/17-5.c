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
 * Test that the policy and scheduling parameters remain unchanged when the
 * policy value is not defined in the sched.h header.
 *
 * The test attempt to set the policy to a very improbable value.
 * Steps:
 *   1. Get the old policy and priority.
 *   2. Call sched_setscheduler with invalid args.
 *   3. Check that the policy and priority have not changed.
 *
 * Orignal author unknown
 * Updated:  Peter W. Morreale <pmorreale AT novell DOT com>
 * Date:     09.07.2011
 */

#include <sched.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define ERR_MSG(f, rc)  printf("Failed: %s rc: %d errno: %s\n", \
				f, rc, strerror(errno))

int main(void)
{
	int old_priority, old_policy, new_policy;
	struct sched_param param;
	int invalid_policy;
	char *label;
	pid_t pid = getpid();
	int rc;

	invalid_policy = INT_MAX;

	label = "sched_getparam()";
	rc = sched_getparam(pid, &param);
	if (rc)
		goto unresolved;
	old_priority = param.sched_priority;

	label = "sched_getscheduler()";
	rc = sched_getscheduler(pid);
	if (rc < 0)
		goto unresolved;
	old_policy = rc;

	label = "sched_setscheduler() - invalid policy succeeded?";
	rc = sched_setscheduler(0, invalid_policy, &param);
	if (!rc)
		goto unresolved;

	label = "sched_getparam()";
	rc = sched_getparam(pid, &param);
	if (rc)
		goto unresolved;

	label = "sched_getscheduler()";
	rc = sched_getscheduler(pid);
	if (rc < 0)
		goto unresolved;
	new_policy = rc;

	if (old_policy != new_policy) {
		printf("Failed: invalid policy change, old: %u, new %u\n",
		       old_policy, new_policy);
		return PTS_FAIL;
	}

	if (old_priority != param.sched_priority) {
		printf("Failed: invalid priority change, old: %u, new %u\n",
		       old_priority, param.sched_priority);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

unresolved:
	ERR_MSG(label, rc);
	return PTS_UNRESOLVED;
}
