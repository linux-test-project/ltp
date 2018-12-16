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
 * Test that sched_getscheduler() sets errno == EPERM when the requesting
 * process does not have permission
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include "posixtest.h"

/* Set the euid of this process to a non-root uid */
int set_nonroot(void)
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
			printf("You don't have permission to change "
			       "your UID.\n");
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

	int result = -1;

	/* We assume process Number 1 is created by root */
	/* and can only be accessed by root */
	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	result = sched_getscheduler(1);

	if (result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	if (result == 0) {
		printf("The function sched_getscheduler has successed.\n");
		return PTS_UNTESTED;
	}
	if (errno != EPERM) {
		perror("errno is not EPERM");
		return PTS_FAIL;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;
	}
}
