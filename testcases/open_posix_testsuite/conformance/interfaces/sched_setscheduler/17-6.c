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
 * requesting process  does not have permission to set the scheduling parameters
 * for the specified process, or does not have the appropriate privilege to
 * invoke sched_setscheduler().
 *
 * Atempt to change the policy of the process whose ID is 1 which is generally
 * belongs to root. This test can not be run by root.
 * Steps:
 *   1. Get the old policy and priority.
 *   2. Call sched_setscheduler with pid arg == 1.
 *   3. Check that the policy and priority have not changed.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include "posixtest.h"

/** Set the euid of this process to a non-root uid */
int set_nonroot()
{
	struct passwd *pw;
	setpwent();
	/* search for the first user which is non root */
	while ((pw = getpwent()) != NULL)
		if (strcmp(pw->pw_name, "root"))
			break;
	endpwent();
	if (pw == NULL) {
		printf("There is no other user than current and root.\n");
		return 1;
	}

	if (seteuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to change your UID.\n");
			return 1;
		}
		perror("An error occurs when calling seteuid()");
		return 1;
	}

	printf("Testing with user '%s' (uid: %d)\n",
	       pw->pw_name, (int)geteuid());
	return 0;
}

int main(void)
{
	int max_priority, old_priority, old_policy, new_policy, policy;
	struct sched_param param;

	/* We assume process Number 1 is created by root */
	/* and can only be accessed by root */
	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	if (sched_getparam(getpid(), &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	old_policy = sched_getscheduler(getpid());
	if (old_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	/* Make sure that policy != old_policy */
	policy = old_policy == SCHED_FIFO ? SCHED_RR : SCHED_FIFO;

	/* Make sure that param.sched_priority != old_priority */
	max_priority = sched_get_priority_max(policy);
	param.sched_priority = (old_priority == max_priority) ?
	    sched_get_priority_min(policy) : max_priority;

	sched_setscheduler(1, policy, &param);

	if (sched_getparam(getpid(), &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	new_policy = sched_getscheduler(getpid());
	if (new_policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}

	if (old_policy == new_policy && old_priority == param.sched_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (param.sched_priority != old_priority) {
		printf("The param has changed\n");
	}
	if (new_policy != old_policy) {
		printf("The policy has changed\n");
	}
	return PTS_FAIL;
}
