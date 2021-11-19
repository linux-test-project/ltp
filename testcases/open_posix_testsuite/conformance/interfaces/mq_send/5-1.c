/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2008, Novell Inc. All rights reserved.
 *
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * Major fixes by: Brandon Philips <bphilips@suse.de>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the message queue is full and O_NONBLOCK is not set, mq_send()
 * will block until it can place the message in the queue.
 *
 * Test by sending messages in a child process until the message queue is full.
 * At this point, the child should be blocking on sending.  Then, have the
 * parent receive the message and return pass when the next message is sent
 * to the message queue.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
 *
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"
#include "mq_send.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 5

static char gqname[NAMESIZE];
static mqd_t gqueue;

static int sync_pipes[2];

static int cleanup_for_exit(int gqueue, char *gqname, int ret)
{
	mq_close(gqueue);
	mq_unlink(gqname);

	if (ret == PTS_PASS)
		printf("Test PASSED\n");
	else if (ret == PTS_FAIL)
		printf("Test FAILED\n");

	return ret;
}

int main(void)
{
	int pid;
	char msgrcd[BUFFER];
	const char *msgptr = MSGSTR;
	struct mq_attr attr;
	unsigned pri;

	sprintf(gqname, "/mq_send_5-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;

	/* Use O_CREAT + O_EXCL to avoid using a previously created queue */
	gqueue =
	    mq_open(gqname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR,
		    &attr);
	if (gqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	if (sync_pipe_create(sync_pipes) == -1) {
		perror("sync_pipe_create() did not return success");
		return cleanup_for_exit(gqueue, gqname, PTS_UNRESOLVED);
	}

	if ((pid = fork()) == 0) {
		/* child here */
		int i;

		for (i = 0; i < MAXMSG + 1; i++) {
			mq_send(gqueue, msgptr, strlen(msgptr), 1);

			if (sync_pipe_notify(sync_pipes) < 0) {
				perror
				    ("sync_pipe_notify() did not return success");
				return cleanup_for_exit(gqueue, gqname,
							PTS_UNRESOLVED);
			}
		}
	} else {
		/* parent here */
		int j;

		/* wait for child to reach MAXMSG */
		for (j = 0; j < MAXMSG; j++) {
			/* set a long timeout since we are expecting success */
			if (sync_pipe_wait_select(sync_pipes, 60) != 0) {
				printf("sync_pipe_wait\n");
				return cleanup_for_exit(gqueue, gqname,
							PTS_FAIL);
			}
		}

		/* if we don't timeout here then we got too many messages, child never blocked */
		if (sync_pipe_wait_select(sync_pipes, 1) != -ETIMEDOUT) {
			printf("Child never blocked\n");
			kill(pid, SIGKILL);	//kill child
			return cleanup_for_exit(gqueue, gqname, PTS_FAIL);
		}

		/* receive one message and allow child's mq_send to complete */
		if (mq_receive(gqueue, msgrcd, BUFFER, &pri) == -1) {
			perror("mq_receive() did not return success");
			return cleanup_for_exit(gqueue, gqname, PTS_UNRESOLVED);
		}

		/* child has 5 seconds to call mq_send() again and notify us */
		if (sync_pipe_wait_select(sync_pipes, 5) == -ETIMEDOUT) {
			/*
			 * mq_send didn't unblock
			 */
			kill(pid, SIGKILL);	//kill child
			printf("mq_send() didn't appear to complete\n");
			return cleanup_for_exit(gqueue, gqname, PTS_FAIL);
		}

		return cleanup_for_exit(gqueue, gqname, PTS_PASS);
	}

	return PTS_UNRESOLVED;
}
