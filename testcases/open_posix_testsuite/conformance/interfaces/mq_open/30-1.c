/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test mq_open() failing with ENOSPC if there is not enough
 * space to create the message queue as system space cannot be controled
 * from this test.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test mq_open() failing with ENOSPC when there\n");
	printf("is not enough space to create the message queue\n");
	printf("as system space cannot be controlled from this test.\n");
	return PTS_UNTESTED;
}