/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Upon successful completion, pthread_attr_destroy() shall return a value of 0.
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Destroy that initialized attribute using pthread_attr_destroy().
 *     This should return 0; 
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"


int main()
{
	pthread_attr_t new_attr;

	/* Initialize attribute */
	if(pthread_attr_init(&new_attr) != 0)
	{
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Destroy attribute */
	if(pthread_attr_destroy(&new_attr) != 0)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	else
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}
}


