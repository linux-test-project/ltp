/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_condattr_init()
 *   shall initialize a condition variable attribute object 'attr' with the default
 *   value for all of the attributes defined by the implementation.

 * Steps:
 * 1.  Initialize a pthread_condattr_t object with pthread_condattr_init()
 * 2.  Call pthread_condattr_getpshared() to check if the process-shared 
 *     attribute is set as the default value PTHREAD_PROCESS_PRIVATE.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include "posixtest.h"

int main()
{
	pthread_condattr_t condattr;
	int rc;

#ifdef PTHREAD_PROCESS_SHARED
	int pshared;
#endif
	/* Initialize a cond attributes object */
	if((rc=pthread_condattr_init(&condattr)) != 0)
	{
		fprintf(stderr,"Error at pthread_condattr_init(), rc=%d\n",rc);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	
#ifdef PTHREAD_PROCESS_SHARED
	/* If the symbol {PTHREAD_PROCESS_SHARED} is defined, the attribute
	 * process-shared should be provided and its default value should be 
	 * PTHREAD_PROCESS_PRIVATE  */
	if(pthread_condattr_getpshared(&condattr, &pshared) != 0)
	{
		fprintf(stderr,"Error obtaining the attribute process-shared\n");
		return PTS_UNRESOLVED;
	}
	
	if(pshared == PTHREAD_PROCESS_PRIVATE)
	{
		printf("Test PASSED\n");
		return PTS_PASS;
	}
	else
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
#endif	
	
	fprintf(stderr,"process-shared attribute is not available for testing\n");
	return PTS_UNRESOLVED;	
}
