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
 * Test that sched_getparam() sets errno == EPERM if the requesting process
 * does not have permission.
 */

 /* adam.li@intel.com - 2004-05-21
  *
  * On Linux, e.g, the kernel makes no check on user permission to call this
  * API. So basically we don't know on what condition a system should return
  * EPERM. It is implementation defined.
  */

#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

/** Set the euid of this process to a non-root uid */
int set_nonroot()
{
	struct passwd *pw;
	setpwent();
	/* search for the first user which is non root */
	while ((pw = getpwent()) != NULL)
		if (pw->pw_uid != 0)
			break;
	endpwent();
	if (pw == NULL) {
		printf("There is no other user than current and root.\n");
		return 1;
	}

	if (setgid(pw->pw_gid) != 0) {
		if (errno == EPERM)
			printf("You don't have permission to change "
			       "your GID.\n");
		else
			perror("setgid failed");
		return 1;
	}

	if (setuid(pw->pw_uid) != 0) {
		if (errno == EPERM)
			printf("You don't have permission to change "
			       "your UID.\n");
		else
			perror("setuid failed");

		return 1;
	}

	printf("Testing with user '%s' (euid,uid) = (%d,%d)\n",
	       pw->pw_name, geteuid(), getuid());
	return 0;
}

int main(void)
{

	struct sched_param param;
	int result = -1;

	/*
	 * We assume process Number 1 is created by root
	 * and can only be accessed by root
	 * This test should be run under standard user permissions
	 */
	if (geteuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	result = sched_getparam(1, &param);

	if (result == -1 && errno == EPERM) {
		printf("Test PASS\n");
		return PTS_PASS;
	}
	if (result == 0) {
		printf("The function sched_getparam has successed.\n");
		return PTS_UNTESTED;
	}
	if (errno != EPERM) {
		perror("errno is not EPERM: The system allows a non-root"
		       "user to use sched_getparam()");
		return PTS_UNRESOLVED;
	} else {
		perror("Unresolved test error");
		return PTS_UNRESOLVED;
	}
}
