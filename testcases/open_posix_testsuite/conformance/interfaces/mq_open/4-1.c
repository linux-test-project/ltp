/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test that if the message queue descriptor is implemented with
 * a file descriptor then at least {OPEN_MAX} file and message queues
 * can be opened as we cannot determine at run-time if a given implementation
 * is implemented with a file descriptor.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test that {OPEN_MAX} file and message queues can\n");
	printf("be opened as we cannot determine at run-time if a given\n");
	printf("implementation is implemented with a file descriptor.\n");
	return PTS_UNTESTED;
}