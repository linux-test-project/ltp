/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that if the signal is the null signal (0), no signal is sent.
 *  1) Call pthread_kill on the current thread with the null signal.
 *  2) If process is still functional after kill() is called, consider
 *     the test a pass (most likely no signal was sent).
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	pthread_t main_thread;

	main_thread=pthread_self();

	if (pthread_kill(main_thread, 0) != 0) {
		printf("Could not call pthread_kill with sig = 0\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}

