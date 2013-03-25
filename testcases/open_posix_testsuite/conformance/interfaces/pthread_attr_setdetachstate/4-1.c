/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_attr_setdetachstate()
 *
 * shall fail if:
 * -[EINVAL]  The value of detachstate was not valid
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setdetachstate() using an invalid value of 0
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int main(void)
{
	pthread_attr_t new_attr;
	int ret_val, invalid_val;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0) {
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Set the attribute object to an invalid value of 1000000. */
	invalid_val = 1000000;
	ret_val = pthread_attr_setdetachstate(&new_attr, invalid_val);

	if (ret_val != EINVAL) {
		printf("Test FAILED: Returned %d instead of EINVAL\n", ret_val);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
