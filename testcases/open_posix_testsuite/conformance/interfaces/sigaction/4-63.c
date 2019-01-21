/*

 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  rusty.lynch REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

  Test case for assertion #4 of the sigaction system call that shows
  that attempting to add SIGKILL to the signal mask of SIGQUIT will
  not result in sigaction returning -1
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
}

int main(void)
{
	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGKILL);
	if (sigaction(SIGQUIT, &act, 0) == -1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
