/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com:wq
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 *  If one or more process have the message queue open, 
 *  mq_unlink() may block until all the referneces have been closed.
 *  NOTE: It is difficult to detect such instance. so won't test.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Difficult to detect whether mq_unlink will block until all the reference have been closed\n");
	printf("for this instance.  Will not be tested.\n");
	return PTS_UNTESTED;
}
