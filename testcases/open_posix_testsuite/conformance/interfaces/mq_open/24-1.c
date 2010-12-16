/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test mq_open() failing with EINTR since mq_open() does not have
 * a blocking situation where we could predictably interrupt it with
 * a signal.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test mq_open() being interrupted as it is\n");
	printf("not possible to predictably interrupt an mq_open().\n");
	return PTS_UNTESTED;
}