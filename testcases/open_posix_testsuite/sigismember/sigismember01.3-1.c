/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Tests assertion 3 by filling a signal set and arbitrarily querying 
 * it for a SIGABRT function. Sigmember should return a 1.
*/

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int main() {

	sigset_t signalset;

	if (sigfillset(&signalset) == -1) {
		perror("sigfillset failed -- test aborted");
		return PTS_UNRESOLVED;
	}

	if (sigismember(&signalset, SIGABRT) != 1) {
		#ifdef DEBUG
			printf("sigismember didn't returned a 1 even though sigfillset was just called\n");
		#endif
		return PTS_FAIL;
	}

	printf("sigismember passed\n");
	return PTS_PASS;
}
