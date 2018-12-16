/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */
/*
   open_sem test case that attempts to open a new semaphore with read
   permissions, close it, then create the same semaphore with write
   permissions which should come up with denial access.
 */


#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_open"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

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
	sem_t *mysemp;
	char semname[50];

	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/*Trying to open the first Sem with read mode */
	mysemp = sem_open(semname, O_CREAT, 0444, 1);

	/* Opening the same existance SEM with write mode */
	mysemp = sem_open(semname, O_CREAT, 0222, 1);

	if (mysemp != SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (errno == EACCES) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
