/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the first letter of name is "/", any other processes
 * calling the message queue with that name will refer to the same
 * message queue object.
 *
 * Test by creating a message queue and placing a message within it.  Then,
 * in another process open the first message queue and ensure the message
 * can be received.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40

#define CHILDPASS 1
#define CHILDFAIL 0

void handler(int signo)
{
	(void) signo;
	return;
}

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	int pid, ret;

	sprintf(qname, "/mq_open_2-1_%d", getpid());

	pid = fork();
	if (pid == 0) {
		mqd_t childqueue;
		char msgrcd[BUFFER];
		sigset_t mask;
		struct mq_attr attr;
		struct sigaction act;
		int sig;
		unsigned pri;

		/* child here */

		/* Set up handler for SIGUSR1 */
		act.sa_handler = handler;
		act.sa_flags = 0;
		sigaction(SIGUSR1, &act, NULL);

		/* wait for parent to finish mq_open */
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		sigwait(&mask, &sig);

		/* now that parent has finished, open qname */
		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		childqueue = mq_open(qname, O_RDWR, S_IRUSR | S_IWUSR, &attr);
		if (childqueue == (mqd_t) - 1) {
			perror("mq_open() did not return success in child");
			return CHILDFAIL;
		}

		ret = mq_receive(childqueue, msgrcd, BUFFER, &pri);
		if (ret == -1) {
			perror("mq_receive() returned failure in child");
			return CHILDFAIL;
		}

		if (strcmp(msgptr, msgrcd) != 0) {
			printf("FAIL:  sent %s received %s\n", msgptr, msgrcd);
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Received message %s\n", msgrcd);
#endif
		return CHILDPASS;
	} else {
		/* parent here */
		mqd_t queue;
		int i;
		struct mq_attr attr;

		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		queue = mq_open(qname, O_CREAT | O_RDWR,
				S_IRUSR | S_IWUSR, &attr);
		if (queue == (mqd_t) - 1) {
			perror("mq_open() did not return success");
			printf("Test UNRESOLVED\n");
			/* stop child and exit */
			kill(pid, SIGABRT);
			return PTS_UNRESOLVED;
		}

		if (mq_send(queue, msgptr, strlen(msgptr) + 1, 1) != 0) {
			perror("mq_send() did not return success");
			printf("Test UNRESOLVED\n");
			/* close queue, stop child and exit */
			mq_close(queue);
			mq_unlink(qname);
			kill(pid, SIGABRT);
			return PTS_UNRESOLVED;
		}

		sleep(1);
		kill(pid, SIGUSR1);

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit");
			/* close queue and exit */
			printf("Test UNRESOLVED\n");
			mq_close(queue);
			mq_unlink(qname);
			return PTS_UNRESOLVED;
		}

		mq_close(queue);
		mq_unlink(qname);

		if (!WIFEXITED(i) || !WEXITSTATUS(i)) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}
