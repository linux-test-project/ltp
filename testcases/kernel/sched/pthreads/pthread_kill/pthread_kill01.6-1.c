/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *  Test that when the null signal is sent to pthread_kill(), error 
    checking is still performed.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "posixtest.h"

int main()
{
	if (ESRCH != pthread_kill(9999999, 0)) {
		printf("pthread_kill() did not fail on ESRCH\n");
		return PTS_FAIL;
	}

	return PTS_PASS;
}
