/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the owner is unchanged when O_TRUNC is set, the shared memory
 * object exists and it is successfully opened O_RDWR.
 *
 * In most case this test will be unresolved if not run by root.
 * Steps:
 *  1. Create a shared memory object.
 *  2. Set a non zero size for this object (to force the modification of the
 *     object when it will be reopen with O_TRUNC set).
 *  3. Set his effective user id to an other user id which is not root.
 *  4. Call shm_open with O_TRUNC set.
 *  5. Check that the owner is unchanged.
 */

/* getpwent() is part of XSI option */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "posixtest.h"

#define BUF_SIZE 8
#define SHM_NAME "posixtest_26-2"

int main(void)
{
	int fd;
	struct stat stat_buf;
	struct passwd *pw;
	uid_t old_uid;
	gid_t old_gid;

	umask(0);
	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT,
		      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
		      S_IWOTH);

	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	if (ftruncate(fd, BUF_SIZE) != 0) {
		perror("An error occurs when calling ftruncate()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (fstat(fd, &stat_buf) != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}
	old_uid = stat_buf.st_uid;
	old_gid = stat_buf.st_gid;

	/* search for the first user which is non root and which is not the
	   current user */
	while ((pw = getpwent()) != NULL)
		if (strcmp(pw->pw_name, "root") && pw->pw_uid != getuid())
			break;

	if (pw == NULL) {
		printf("There is no other user than current and root.\n");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (seteuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to change your UID.\nTry to rerun this test as root.\n");
		} else {
			perror("An error occurs when calling seteuid()");
		}
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	printf("Testing with user '%s' (uid: %i)\n", pw->pw_name, pw->pw_uid);

	fd = shm_open(SHM_NAME, O_RDWR | O_TRUNC, 0);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		if (seteuid(getuid()))
			perror("seteuid");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (fstat(fd, &stat_buf) != 0) {
		perror("An error occurs when calling fstat()");
		if (seteuid(getuid()))
			perror("seteuid");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	if (seteuid(getuid()))
		perror("seteuid");
	shm_unlink(SHM_NAME);

	if (stat_buf.st_uid == old_uid && stat_buf.st_gid == old_gid) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (stat_buf.st_uid != old_uid)
		printf("The user ID has changed.\n");
	if (stat_buf.st_gid != old_gid)
		printf("The group ID has changed.\n");
	return PTS_FAIL;
}
