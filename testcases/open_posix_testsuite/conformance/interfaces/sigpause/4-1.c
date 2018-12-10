/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that sigpause() returns -1 and sets errno to EINVAL
 if passed an invalid signal number.

 */


#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

#if 0 && defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
int main(void)
{
	printf("Function definition doesn't match POSIX definition "
	       "and preceded POSIX definition; interface is obsolete\n");
	return PTS_UNSUPPORTED;
}
#else
int main(void)
{
	int return_value = 0;
	int result;

	return_value = sigpause(-1);
	if (return_value == -1) {
		if (errno == EINVAL) {
			printf("Test PASSED: sigpause returned -1 and "
			       "set errno to EINVAL\n");
			result = 0;
		} else {
			printf("Test FAILED: sigpause did not set errno "
			       "to EINVAL\n");
			result = 1;
		}
	} else {
		printf("Test FAILED: sigpause did not return -1\n");
		if (errno == EINVAL)
			printf("Test FAILED: sigpause did not set errno "
			       "to EINVAL\n");
		result = 1;
	}

	if (result == 2)
		return PTS_UNRESOLVED;
	if (result == 1)
		return PTS_FAIL;

	printf("Test PASSED\n");
	return PTS_PASS;
}
#endif
