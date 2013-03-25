/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the access mode is O_RDWR, the message queue can
 * send messages and receive messages.
 *
 * Test for a message queue opened twice in the same process.
 *
 * Note:  Not testing that the message sent/received is the one expected
 * (that would be part of mq_send(), mq_receive() testing).  Just testing
 * it can be sent/received.
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
	char qname[NAMESIZE], msgrcd[BUFFER], msgrcd2[BUFFER];
	const char *msgptr = MSGSTR;
	mqd_t rdwrqueue, rdwrqueue2;
	struct mq_attr attr;
	unsigned pri;

	sprintf(qname, "/mq_open_9-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	rdwrqueue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (rdwrqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success on read-write queue");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	if (mq_send(rdwrqueue, msgptr, strlen(msgptr), 1) == -1) {
		perror("mq_send() did not return success");
		printf("Test FAILED\n");
		/* close queue and exit */
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message %s sent\n", msgptr);
#endif

	if (mq_receive(rdwrqueue, msgrcd, BUFFER, &pri) == -1) {
		perror("mq_receive() did not return success");
		printf("Test FAILED\n");
		/* close queue and exit */
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message %s received\n", msgrcd);
#endif

	rdwrqueue2 = mq_open(qname, O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (rdwrqueue2 == (mqd_t) - 1) {
		perror("mq_open() did not return success on read-write queue");
		/* close rdwrqueue and exit */
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	if (mq_send(rdwrqueue2, msgptr, strlen(msgptr), 1) == -1) {
		perror("mq_send() did not return success");
		printf("Test FAILED\n");
		/* close queues and exit */
		mq_close(rdwrqueue);
		mq_close(rdwrqueue2);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message %s sent to second queue\n", msgptr);
#endif

	if (mq_receive(rdwrqueue2, msgrcd2, BUFFER, &pri) == -1) {
		perror("mq_receive() did not return success");
		/* close queues and exit */
		mq_close(rdwrqueue);
		mq_close(rdwrqueue2);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message %s received\n", msgrcd2);
#endif

	mq_close(rdwrqueue);
	mq_close(rdwrqueue2);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
