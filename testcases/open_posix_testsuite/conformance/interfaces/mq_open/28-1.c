/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test returning with ENFILE if the system has too many
 * message queues open when mq_open() is called as we do not have
 * control of the system's message queues from this test.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test returning with ENFILE if the system has\n");
	printf("too many message queues as this is beyond this\n");
	printf("test's domain.\n");
	return PTS_UNTESTED;
}