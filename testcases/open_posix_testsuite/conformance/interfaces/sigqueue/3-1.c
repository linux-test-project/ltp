/*   
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program tests the assertion that if the user id of the sending process
 doesn't match the user id of the receiving process (pid), then the sigqueue
 function will fail with errno set to EPERM, unless unless the sending process
 has appropriate privileges.

 Since process pid 1 (init) is not killable by even root, it is used as a the receiving
 process. Even if process id 1 is killable, this test is still safe because the
 value of the sig parameter is 0.

 */

#define _XOPEN_REALTIME 1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{

	union sigval value;
	value.sival_int = 0;	/* 0 is just an arbitrary value */

        /* We assume process Number 1 is created by root */
        /* and can only be accessed by root */
        /* This test should be run under standard user permissions */
        if (getuid() == 0) {
                puts("Run this test case as a Regular User, but not ROOT");
                return PTS_UNTESTED;
        }

	if (sigqueue(1, 0, value) != -1) {
		printf("Test FAILED: sigqueue() succeeded even though this program's user id did not match the recieving process's user id\n");
		return PTS_FAIL;
	}

	if (EPERM != errno) {
		printf("Test FAILED: EPERM error not received\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}

