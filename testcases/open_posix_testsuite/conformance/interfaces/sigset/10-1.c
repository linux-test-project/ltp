/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program tests the assertion that when unsuccessful, SIG_ERROR
 shall be returned, and errno set to EINVAL.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	if (sigset(SIGKILL, SIG_IGN) == SIG_ERR) {
		if (errno != EINVAL) {
			printf
			    ("Test FAILED: sigset() returned SIG_ERR but didn't set errno to EINVAL\n");
			return PTS_FAIL;
		}
	} else {
		printf
		    ("Test FAILED: sigset() didn't return SIG_ERROR even though SIGKILL was passed to it\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
