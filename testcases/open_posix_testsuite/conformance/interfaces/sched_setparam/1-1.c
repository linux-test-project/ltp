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
 * Test that sched_setparam() sets the scheduling parameters to the parameters
 * specified in the sched_param structure pointed to by param.
 */

#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

void child_proc()
{
	sigset_t signalset;
	int sig;

	if (sigemptyset(&signalset) != 0) {
		perror("An error occurs when calling sigemptyset()");
		exit(1);
	}
	if (sigaddset(&signalset, SIGUSR1) != 0) {
		perror("An error occurs when calling sigaddset()");
		exit(1);
	}
	if (sigwait(&signalset, &sig) != 0) {
		perror("An error occurs when calling sigwait()");
		exit(1);
	}

	exit(0);
}

int main(void)
{
	int result, child_pid, tmp_errno, policy;
	int min_priority, new_priority, old_priority;
	struct sched_param param;

	/* Create a child process which wait SIGUSR1 */
	child_pid = fork();
	if (child_pid == -1) {
		perror("An error occurs when calling fork()");
		return PTS_UNRESOLVED;
	} else if (child_pid == 0) {
		child_proc();
	}

	if (sched_getparam(child_pid, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		kill(child_pid, SIGUSR1);
		return PTS_UNRESOLVED;
	}

	/* Assume that the process have permission to change priority of
	   its child */
	old_priority = param.sched_priority;

	policy = sched_getscheduler(0);
	if (policy == -1) {
		perror("An error occurs when calling sched_getscheduler()");
		return PTS_UNRESOLVED;
	}
	min_priority = sched_get_priority_min(policy);

	new_priority = param.sched_priority == min_priority ?
	    (param.sched_priority = sched_get_priority_max(policy)) :
	    (param.sched_priority = min_priority);

	result = sched_setparam(child_pid, &param);
	tmp_errno = errno;

	if (sched_getparam(child_pid, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		kill(child_pid, SIGUSR1);
		return PTS_UNRESOLVED;
	}

	if (result == 0 && param.sched_priority == new_priority) {
		printf("Test PASSED\n");
		kill(child_pid, SIGUSR1);
		return PTS_PASS;
	} else if (result == 0 && param.sched_priority == old_priority) {
		printf("The param does not change.\n");
		kill(child_pid, SIGUSR1);
		return PTS_FAIL;
	} else if (result == -1 && tmp_errno == EPERM) {
		printf
		    ("The process have not permission to change the param of its child.\n");
		kill(child_pid, SIGUSR1);
		return PTS_UNRESOLVED;
	} else if (result == -1 && tmp_errno == EINVAL) {
		/* the new priority may be to big */
		/* test with a new priority lower than the old one */
		param.sched_priority = (new_priority -= 2);
		result = sched_setparam(child_pid, &param);

		if (result == 0 && param.sched_priority == new_priority) {
			printf("Test PASSED");
			kill(child_pid, SIGUSR1);
			return PTS_PASS;
		}
	}

	perror("Unknow error");
	kill(child_pid, SIGUSR1);
	return PTS_FAIL;
}
