/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Since it is undefined what will happen if mqdes is used after
 * mq_close() is successfully called and mq_open() has not been
 * called again, this will not be tested.  (Eventually, this could
 * be a speculative test)
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Functionality of using mqdes after mq_close() and before\n");
	printf("mq_open() will not be tested as POSIX says this is ");
	printf("undefined.\n");
	return PTS_UNTESTED;
}