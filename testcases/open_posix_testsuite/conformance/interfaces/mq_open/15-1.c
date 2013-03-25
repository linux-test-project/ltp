/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_EXCL and O_CREAT are set and message queue name already
 * exists, mq_open() fails.
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
	mqd_t queue, queue2;

	sprintf(qname, "/mq_open_15-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	/*
	 * Open queue qname again with O_CREAT and O_EXCL set
	 */
	queue2 = mq_open(qname, O_CREAT | O_EXCL | O_RDWR,
			 S_IRUSR | S_IWUSR, NULL);
	if (queue2 != (mqd_t) - 1) {
		printf("mq_open() should have failed with O_CREAT and\n");
		printf("O_EXCL on an already opened queue.\n");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_close(queue2);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
