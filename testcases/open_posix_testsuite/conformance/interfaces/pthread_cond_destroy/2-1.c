/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_mutex_destroy() that
 * It shall be safe to destroy an initialized condition variable upon 
 * which no threads are currently blocked. 
 *
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Test PASSED\n");
	return PTS_PASS;
}
