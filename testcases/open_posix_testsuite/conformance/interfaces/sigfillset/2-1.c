/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 Simply, as long as sigfillset returns a 0, the test passes.
*/

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main(void)
{
	sigset_t signalset;
/*
	int returnval;

	returnval = sigfillset(&signalset);

	if (returnval != 0) {
*/
	if (sigfillset(&signalset) != 0) {
		perror("sigfillset failed -- test aborted");
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("sigfillset passed\n");
#endif
	return PTS_PASS;
}
