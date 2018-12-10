/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_mutexattr_gettype()
 *
 * It shall fail if:
 *
 * [EINVAL] - The value specified by 'attr' is invalid.
 *
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutexattr_gettype() with an invalid 'attr'.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "posixtest.h"
#include <errno.h>

int main(void)
{
	pthread_mutexattr_t mta;
	int type, ret;

	/* Make 'attr' invalid by not initializing it and using memset. */
	memset(&mta, 0, sizeof(mta));

	/* Pass an invalid 'attr'.  */
	ret = pthread_mutexattr_gettype(&mta, &type);

	switch (ret) {
	case 0:
		printf("UNTESTED: The call didn't fail.\n");
		return PTS_UNTESTED;
	case EINVAL:
		printf("Test PASSED\n");
		return PTS_PASS;
	default:
		printf("Test FAILED: Expected EINVAL, but got: %d\n", ret);
		return PTS_FAIL;
	}
}
