/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test that oflag permissions behave just as file permissions.
 * Assertions 6-9 cover more of this.  Since it's TBD the permissions
 * of the calling program, assertion 5 seems difficult to test.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test oflag permissions behave as file permissions\n");
	printf("for a process.  Other assertions test oflags more.\n");
	return PTS_UNTESTED;
}

