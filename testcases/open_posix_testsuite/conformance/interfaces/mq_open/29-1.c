/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_open() fails with ENOENT if the named message queue does
 * not yet exist, but O_CREAT is not set.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define NAMESIZE 50

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;

	sprintf(qname, "/mq_open_29-1_%d", getpid());

	queue = mq_open(qname, O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue != (mqd_t) - 1) {
		printf("mq_open() did not return mqd_t - on error\n");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	if (errno != ENOENT) {
		printf("errno != ENOENT\n");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
