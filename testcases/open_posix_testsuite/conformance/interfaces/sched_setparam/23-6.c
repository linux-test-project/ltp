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
 * Test that the priority remain unchanged when the requesting process
 * does not have permission to set the scheduling parameters for the specified
 * process, or does not have the appropriate privilege to invoke schedparam().
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
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

	if (setuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to change your UID.\n");
			return 1;
		}
		perror("An error occurs when calling seteuid()");
		return 1;
	}

	printf("Testing with user '%s' (euid: %d)(uid: %d)\n",
	       pw->pw_name, (int)geteuid(), (int)getuid());
	return 0;
}

int main(void)
{
	int old_priority;
	struct sched_param param;
	int rc;

	/* We assume process Number 1 is created by root */
	/* and can only be accessed by root */
	/* This test should be run under standard user permissions */

	param.sched_priority = sched_get_priority_min(SCHED_FIFO);
	/* Cannot test SCHED_OTHER, on 0 valid as its priority */
	sched_setscheduler(0, SCHED_FIFO, &param);
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	if (sched_getparam(0, &param) == -1) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}
	old_priority = param.sched_priority;

	param.sched_priority++;
	rc = sched_setparam(0, &param);
	if (rc != -1 || (rc == -1 && errno != EPERM)) {
		perror("sched_setparam() does not return EPERM\n");
		return PTS_UNRESOLVED;
	}

	if (sched_getparam(0, &param) != 0) {
		perror("An error occurs when calling sched_getparam()");
		return PTS_UNRESOLVED;
	}

	if (param.sched_priority == old_priority) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("The priority have changed.\n");
		return PTS_FAIL;
	}
}
