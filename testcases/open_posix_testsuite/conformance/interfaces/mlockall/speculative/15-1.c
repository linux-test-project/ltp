/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlockall() function set errno = EPERM if the calling process
 * does not have the appropriate privilege to perform the requested operation
 *
 * It is a may assertion.
 */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <sys/resource.h>
#include "posixtest.h"

/** Set the euid of this process to a non-root uid */
int set_nonroot()
{
	struct passwd *pw;
	struct rlimit rlim;
	int ret = 0;

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

	rlim.rlim_cur = 0;
	rlim.rlim_max = 0;
	if ((ret = setrlimit(RLIMIT_MEMLOCK, &rlim)) != 0)
		printf("Failed at setrlimit() return %d \n", ret);

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
	int result;

	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	result = mlockall(MCL_CURRENT);

	if (result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == 0) {
		printf("You have the right to call mlockall\n");
		return PTS_UNRESOLVED;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}
}
