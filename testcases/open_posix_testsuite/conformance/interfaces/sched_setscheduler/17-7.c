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
 * Test that the policy and scheduling parameters remain unchanged when no
 * process can be found corresponding to that specified by pid.
 *
 * The test create a child process which exit immediately and call
 * sched_setscheduler with the pid of defunct child.
 * Steps:
 *   1. Get the old policy and priority.
 *   2. Create a child process which exit immediately.
 *   3. Wait for child to exit.
 *   4. Call sched_setscheduler with the pid of defunct child.
 *   5. Check that the policy and priority have not changed.
 */
#include <sched.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "posixtest.h"

int main(void)
{
	int max_priority, old_priority, old_policy, new_policy, policy;
	int child_pid, stat_loc;
	struct sched_param param;

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

	/* Create a child process which exit immediately */
	child_pid = fork();
	if (child_pid == -1) {
		perror("An error occurs when calling fork()");
		return PTS_UNRESOLVED;
	} else if (child_pid == 0) {
		exit(0);
	}

	/* Wait for the child process to exit */
	if (wait(&stat_loc) == -1) {
		perror("An error occurs when calling wait()");
		return PTS_UNRESOLVED;
	}

	/* Assume the pid is not yet reatributed to an other process */
	sched_setscheduler(child_pid, policy, &param);

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
