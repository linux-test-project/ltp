/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_CREAT is set and attr == NULL, implementation defined
 * attributes are used.
 *
 * Just test that mq_getattr() can be called after mq_open() is called
 * with attr == NULL.
 *
 * Otherwise, this really cannot be tested as attributes are implementation-
 * defined.
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
	struct mq_attr attr;

	sprintf(qname, "/mq_open_12-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_getattr(queue, &attr) != 0) {
		perror("mq_getattr() failed");
		printf("Test FAILED -- could not get attributes\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
