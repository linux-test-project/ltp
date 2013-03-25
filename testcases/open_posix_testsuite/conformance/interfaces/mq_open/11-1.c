/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_CREAT is set and name has already been used to create
 * a currently existing message queue, then the current call has no effect.
 *
 * Just test by calling O_CREAT again and ensure mq_send works again (i.e.,
 * Basically just testing that using O_CREAT twice doesn't break anything.
 * Otherwise, this assertion is basically untestable.)
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
#define MSGSTR "0123456789"

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	mqd_t queue, queue2;

	sprintf(qname, "/mq_open_11-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_send(queue, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() first time did not return success");
		printf("Test UNRESOLVED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	/*
	 * Second call should have no effect
	 */
	queue2 = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue2 == (mqd_t) - 1) {
		perror("mq_open() second time did not return success");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	if (mq_send(queue2, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success second time");
		printf("Test FAILED\n");
		mq_close(queue);
		mq_close(queue2);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_close(queue2);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
