/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Basic test that if the queue already has room, then mq_timedsend()
 * does not fail.
 *
 * Test by calling mq_timedsend() with an already expired timeout value
 * on an empty queue.  Should succeed.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	struct timespec ts;
	mqd_t queue;

	sprintf(qname, "/mq_timedsend_18-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL) - 1;
	ts.tv_nsec = 0;
	if (mq_timedsend(queue, msgptr, strlen(msgptr), 1, &ts) != 0) {
		perror("mq_timedsend() did not return success on empty queue");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
