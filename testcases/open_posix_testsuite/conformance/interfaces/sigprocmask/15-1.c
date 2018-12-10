/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Simply, if sigignore returns a 0 here, test passes.

*/


#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t set;
	sigaddset(&set, SIGABRT);

	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		perror("sigprocmask failed -- returned -- test aborted");
		return PTS_FAIL;
	}
	printf("Test PASSED\n");
	return PTS_PASS;
}
