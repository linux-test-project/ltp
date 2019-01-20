/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
* Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

* This sample test aims to check the following assertion:
*
* The opened message queue descriptors are copied to the child process and
* refer to the same object.

* The steps are:
* -> Open a message queue descriptor.
* -> Send a message to this descriptor.
* -> Fork
* -> Check if that the child's message count for this descriptor is 1.
* -> Unlink the message queue otherwise it will remain in the system.

* The test fails if the child reports 0 message count
*  or if it fails to read the descriptor.

*/


#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "posixtest.h"

static const char *queue_name = "/fork_19_1_mq";
static const char message[] = "I'm your father...";

int main(void)
{
	int ret, status;
	pid_t child, ctl;

	mqd_t mq;
	char rcv[sizeof(message)];

	struct mq_attr mqa;

	/* Create a message queue descriptor */
	mqa.mq_maxmsg = 2;
	mqa.mq_msgsize = sizeof(message);

	mq = mq_open(queue_name, O_RDWR | O_CREAT | O_NONBLOCK,
		     S_IRUSR | S_IWUSR, &mqa);

	if (mq == (mqd_t)-1) {
		perror("Failed to create the message queue descriptor");
		return PTS_UNRESOLVED;
	}

	/* Send 1 message to this message queue */
	ret = mq_send(mq, message, sizeof(message), 0);

	if (ret != 0) {
		mq_close(mq);
		mq_unlink(queue_name);
		perror("Failed to send the message");
		return PTS_UNRESOLVED;
	}

	/* Check the message has been queued */
	ret = mq_getattr(mq, &mqa);

	if (ret != 0) {
		mq_close(mq);
		mq_unlink(queue_name);
		perror("Failed to get message queue attributes");
		return PTS_UNRESOLVED;
	}

	if (mqa.mq_curmsgs != 1) {
		mq_close(mq);
		mq_unlink(queue_name);
		printf("The queue information does not show the new message");
		return PTS_UNRESOLVED;
	}

	/* Create the child */
	child = fork();

	if (child == -1) {
		mq_close(mq);
		mq_unlink(queue_name);
		perror("Failed to fork");
		return PTS_UNRESOLVED;
	}

	/* child */
	if (child == 0) {
		ret = mq_getattr(mq, &mqa);

		if (ret != 0) {
			perror
			    ("Failed to get message queue attributes in child");
			return PTS_FAIL;
		}

		if (mqa.mq_curmsgs != 1) {
			perror
			    ("The queue information does not show the message in child");
			return PTS_FAIL;
		}

		/* Now, receive the message */
		ret = mq_receive(mq, rcv, sizeof(rcv), NULL);

		/* expected message size */
		if (ret != sizeof(message)) {
			perror("Failed to receive the message");
			return PTS_UNRESOLVED;
		}

		printf("Received message: %s\n", rcv);

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		mq_close(mq);
		mq_unlink(queue_name);
		perror("Waitpid returned the wrong PID");
		return PTS_UNRESOLVED;
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		mq_close(mq);
		mq_unlink(queue_name);
		printf("Child exited abnormally");
		return PTS_FAIL;
	}

	/* Check the message has been unqueued */
	ret = mq_getattr(mq, &mqa);

	if (ret != 0) {
		mq_close(mq);
		mq_unlink(queue_name);
		perror("Failed to get message queue attributes the 2nd time");
		return PTS_UNRESOLVED;
	}

	if (mqa.mq_curmsgs != 0) {
		mq_close(mq);
		mq_unlink(queue_name);
		printf("The message received in child was not dequeued.");
		return PTS_FAIL;
	}

	mq_close(mq);
	mq_unlink(queue_name);

	printf("Test passed\n");
	return PTS_PASS;
}
