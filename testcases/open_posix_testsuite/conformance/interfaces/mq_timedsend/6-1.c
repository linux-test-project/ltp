/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Since the implementation of the order in which items are placed into
 * a message queue is undetermined unless Priority Scheduling is enabled,
 * this test will not be written right now.  It needs priority scheduling
 * to complete.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Priority Scheduling needed to make a reliable test case\n");
	printf("for this instance.  Will not be tested.\n");
	return PTS_UNTESTED;
}