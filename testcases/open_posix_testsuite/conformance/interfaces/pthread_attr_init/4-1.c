/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_attr_init()
 * Upon successful completion, pthread_attr_init() shall return a value of 0.

 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  ENOMEM is the only error it returns, so if it doesn't return that error,
 *     the return number should be 0. 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main()
{
	pthread_attr_t new_attr;
	int ret;

	/* Initialize attribute */
	ret=pthread_attr_init(&new_attr);
	if(ret == 0)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	/* There's insufficient memory, can't run test */
	else if(ret == ENOMEM)
	{
		perror("Error in pthread_attr_init()\n");
		return PTS_UNRESOLVED;
	}

	/* Any other return value other than 0 or ENOMEM, means the test
	 * failed, because those are the only 2 return values for this 
	 * function. */
	else 
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

}
