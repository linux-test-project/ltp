/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Call killpg on the current process with 0 as the signal number. If killpg
 returns success, then we pass.
 */


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main(void)
{
	if (killpg(getpgrp(), 0) != 0) {
		printf("Could not call killpg with sig = 0\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
