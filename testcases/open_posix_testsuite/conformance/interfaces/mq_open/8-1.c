/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the access mode is O_WRONLY, the message queue can
 * send messages but not receive.
 *
 * Test for a message queue opened twice in the same process.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
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
#define BUFFER 40

int main(void)
{
	char qname[NAMESIZE], msgrcd[BUFFER];
	const char *msgptr = MSGSTR;
	mqd_t woqueue, woqueue2;
	struct mq_attr attr;
	unsigned pri;

	sprintf(qname, "/mq_open_8-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	woqueue = mq_open(qname, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &attr);
	if (woqueue == (mqd_t) - 1) {
		perror("mq_open() for write-only queue did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	if (mq_send(woqueue, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success");
		printf("Test UNRESOLVED\n");
		/* close queue and exit */
		mq_close(woqueue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
#ifdef DEBUG
	printf("Message %s sent\n", msgptr);
#endif

	if (mq_receive(woqueue, msgrcd, BUFFER, &pri) != -1) {
		printf("mq_receive() returned success on write only queue\n");
		printf("Test FAILED\n");
		/* close queue and exit */
		mq_close(woqueue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message receive failed, as expected\n");
#endif

	woqueue2 = mq_open(qname, O_WRONLY, S_IRUSR | S_IWUSR, &attr);
	if (woqueue2 == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		/* close woqueue and exit */
		mq_close(woqueue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	if (mq_send(woqueue2, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success");
		printf("Test UNRESOLVED\n");
		/* close queues and exit */
		mq_close(woqueue);
		mq_close(woqueue2);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
#ifdef DEBUG
	printf("Message %s sent to second queue\n", msgptr);
#endif

	if (mq_receive(woqueue2, msgrcd, BUFFER, &pri) != -1) {
		printf("mq_receive() returned success on write only queue\n");
		printf("Test FAILED\n");
		/* close queues and exit */
		mq_close(woqueue);
		mq_close(woqueue2);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message receive failed, as expected, on second queue\n");
#endif

	mq_close(woqueue);
	mq_close(woqueue2);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
