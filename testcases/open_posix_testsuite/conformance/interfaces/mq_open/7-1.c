/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the access mode is O_RDONLY, the message queue can
 * receive messages but not send.
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
	mqd_t rdwrqueue, roqueue;
	struct mq_attr attr;
	unsigned pri;

	sprintf(qname, "/mq_open_7-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	rdwrqueue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (rdwrqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	if (mq_send(rdwrqueue, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success");
		printf("Test UNRESOLVED\n");
		/* close queue and exit */
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
#ifdef DEBUG
	printf("Message %s sent\n", msgptr);
#endif

	roqueue = mq_open(qname, O_RDONLY, S_IRUSR | S_IWUSR, &attr);
	if (roqueue == (mqd_t) - 1) {
		perror("mq_open() for read-only queue did not return success");
		printf("Test UNRESOLVED\n");
		/* close read-write queue and exit */
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
#ifdef DEBUG
	printf("read-only message queue opened\n");
#endif

	if (mq_receive(roqueue, msgrcd, BUFFER, &pri) == -1) {
		perror("mq_receive() failed on read-only queue");
		printf("Test FAILED\n");
		/* close queues and exit */
		mq_close(roqueue);
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message received\n");
#endif

	if (mq_send(roqueue, msgptr, strlen(msgptr), 1) == 0) {
		printf("mq_send() succeeded on read-only queue\n");
		printf("Test FAILED\n");
		/* close queues and exit */
		mq_close(roqueue);
		mq_close(rdwrqueue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("Message could not be sent, as expected\n");
#endif

	mq_close(rdwrqueue);
	mq_close(roqueue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
