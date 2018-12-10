/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the mlock() function sets errno = EPERM if the calling process
 * does not have the appropriate privilege to perform the requested operation
 * (Linux 2.6.9 and later) and its RLIMIT_MEMLOCK soft resource limit set to 0.
 */

#define _GNU_SOURCE 1		/* XXX: Read baloney below about CAP_* */

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <sys/resource.h>
#include "posixtest.h"

#define BUFSIZE 8

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

	/*
	 * mlock()
	 * EPERM:
	 * (Linux 2.6.9 and later) the caller was not privileged (CAP_IPC_LOCK)
	 * and its RLIMIT_MEMLOCK soft resource limit was 0.
	 */

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
	void *ptr;

	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	ptr = malloc(BUFSIZE);
	if (ptr == NULL) {
		printf("Can not allocate memory.\n");
		return PTS_UNRESOLVED;
	}

	result = mlock(ptr, BUFSIZE);

	if (result == -1 && errno == EPERM) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else if (result == 0) {
		printf("You have the right to call mlock\n");
		return PTS_FAIL;
	} else {
		perror("Unexpected error");
		return PTS_UNRESOLVED;
	}

}
