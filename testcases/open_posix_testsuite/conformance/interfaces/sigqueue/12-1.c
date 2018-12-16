/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 *  Test that when the process does not have the appropriate privilege
    to send the signal to the receiving process, then sigqueue()
    returns -1 and errno is set to [EPERM]

    The real or effective user ID of the sending process shall match
    the real or saved set-user-ID of the receiving process.
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
	/* setuid will change uid, euid */
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
	int failure = 0;
	union sigval value;
	value.sival_int = 0;	/* 0 is just an arbitrary value */

	/* We assume process Number 1 is created by root */
	/* and can only be accessed by root */
	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	if (-1 == sigqueue(1, 0, value)) {
		if (EPERM == errno) {
			printf("EPERM error received\n");
		} else {
			printf
			    ("sigqueue() failed but errno not set correctly\n");
			failure = 1;
		}
	} else {
		printf("sigqueue() did not return -1\n");
		failure = 1;
	}

	if (failure) {
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
