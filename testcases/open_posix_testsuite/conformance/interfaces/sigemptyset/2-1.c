/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Simply, if sigemptyset returns a 0 here, test passes.

*/

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;

	if ((int)sigemptyset(&signalset) != 0) {
		perror("sigemptyset failed -- returned -- test aborted");
		return PTS_FAIL;
	}
	printf("sigemptyset passed\n");
	return PTS_PASS;
}
