/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that shm_unlink() sets errno = EACCES if permission is denied to unlink
 * the named shared memory object.
 *
 * Steps:
 *  1. Create a shared memory object.
 *  2. Set his effective user id to an other user id which is not root.
 *  3. Try to unlink the name.
 *     If it fail: set the effective user id to real user id and unlink.
 * In most case this test will be unresolved if not run by root.
 */

/* getpwent() is part of XSI option */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_9-1"

int main(void)
{
	int fd, result;
	struct passwd *pw;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	/* search for the first user which is non root and which is not the
	   current user */
	while ((pw = getpwent()) != NULL)
		if (strcmp(pw->pw_name, "root") && pw->pw_uid != getuid())
			break;

	if (pw == NULL) {
		printf("There is no other user than current and root.\n");
		return PTS_UNRESOLVED;
	}

	if (seteuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to change your UID.\nTry to rerun this test as root.\n");
			return PTS_UNRESOLVED;
		}
		perror("An error occurs when calling seteuid()");
		return PTS_UNRESOLVED;
	}

	printf("Testing with user '%s' (uid: %i)\n", pw->pw_name, pw->pw_uid);

	result = shm_unlink(SHM_NAME);
	if (result == -1 && errno == EACCES) {
		printf("Test PASSED\n");
		seteuid(getuid());
		shm_unlink(SHM_NAME);
		return PTS_PASS;
	} else if (result == -1) {
		perror("Unexpected error");
		seteuid(getuid());
		shm_unlink(SHM_NAME);
		return PTS_FAIL;
	}

	printf("shm_unlink() success.\n");
	return PTS_UNRESOLVED;

}
