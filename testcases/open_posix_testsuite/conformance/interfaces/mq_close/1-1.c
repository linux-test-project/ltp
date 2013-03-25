/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_close test case that attempts to open a new message queue,
 *  close the message queue and verify that mq_close returns 0.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "mq_close"

int main(void)
{
	char qname[50];
	mqd_t queue;

	sprintf(qname, "/" FUNCTION "_" TEST "_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	if (mq_close(queue) != 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
