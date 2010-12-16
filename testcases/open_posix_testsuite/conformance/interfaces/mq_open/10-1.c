/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test the user ID and group ID of the message queue as would
 * need multiple users/groups for setup to test.
 *
 * Will also not test the file permissions as the implementation of the
 * message queue is TBD and so it would not be possible to know how to
 * check the file permissions.
 *
 * Will not test what happens if non-file permission bits are set as that
 * is undefined.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test the user ID and group ID of a created\n");
	printf("message queue as we would need multiple users and\n");
	printf("groups on the system to test.\n");
	printf("Will not test the file permissions as testing would\n");
	printf("be implementation defined.\n");
	return PTS_UNTESTED;
}