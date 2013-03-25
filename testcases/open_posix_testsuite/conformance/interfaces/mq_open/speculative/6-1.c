/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Brief test that oflag is a bitwise-inclusive OR of only one of
 * O_RDONLY, O_WRONLY, O_RDWR.  Other aspects of assertion 6 are tested
 * in subsequent assertions (7-9).
 *
 * Moved to speculative/ as behavior for invalid flags isn't really
 * defined by POSIX.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define NAMESIZE 50

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;

	sprintf(qname, "/msgqueue_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR | O_WRONLY,
			S_IRUSR | S_IWUSR, NULL);
	if (queue != (mqd_t) - 1) {
		printf("In this implementation, mq_open() does not fail\n");
		printf("on invalid flags\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_PASS;
	}

	printf("In this implementation, mq_open() fails on invalid flags\n");
	return PTS_PASS;
}
