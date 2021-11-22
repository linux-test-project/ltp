/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the named shared memory object is not changed by this function
 * call when the process does not have permission to unlink the name.
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
#define BUF_SIZE 8

int main(void)
{
	int fd, result;
	struct passwd *pw;
	struct stat stat_before, stat_after;

	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, BUF_SIZE) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (fstat(fd, &stat_before) != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
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
	if (result == 0) {
		printf("shm_unlink() success.\n");
		return PTS_UNRESOLVED;
	}

	if (seteuid(getuid()))
		perror("seteuid");

	if (fstat(fd, &stat_after) != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (stat_after.st_uid != stat_before.st_uid ||
	    stat_after.st_gid != stat_before.st_gid ||
	    stat_after.st_size != stat_before.st_size ||
	    stat_after.st_mode != stat_before.st_mode) {
		printf("The shared memory object has changed.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
