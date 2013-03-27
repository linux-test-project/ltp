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
 * Test for a message queue opened twice in two different processes.
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
	int pid;

	sprintf(qname, "/mq_open_8-2_%d", getpid());

	pid = fork();
	if (pid == 0) {
		mqd_t woqueue;
		sigset_t mask;
		struct mq_attr attr;
		struct sigaction act;
		char msgrcd[BUFFER];
		int sig;
		unsigned pri;

		/* child here */

		/* Set up handler for SIGUSR1 */
		act.sa_handler = handler;
		act.sa_flags = 0;
		sigaction(SIGUSR1, &act, NULL);

		/* wait for parent to finish mq_send */
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		sigwait(&mask, &sig);

		/* once parent has finished mq_send, open new queue */
		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		woqueue = mq_open(qname, O_WRONLY, S_IRUSR | S_IWUSR, &attr);
		if (woqueue == (mqd_t) - 1) {
			perror("mq_open() read only failed in child");
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("write-only message queue opened in child\n");
#endif

		if (mq_receive(woqueue, msgrcd, BUFFER, &pri) != -1) {
			printf("mq_receive() ret success w write-only queue\n");
			mq_close(woqueue);
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Receiving message failed in child, as expected\n");
#endif

		if (mq_send(woqueue, msgptr, strlen(msgptr), 1) == -1) {
			perror("mq_send() failed on a write-only queue");
			mq_close(woqueue);
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Message %s sent in child\n", msgptr);
#endif
		mq_close(woqueue);

		return CHILDPASS;
	} else {
		/* parent here */
		mqd_t woqueue;
		char msgrcd[BUFFER];
		struct mq_attr attr;
		int i;
		unsigned pri;

		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		woqueue = mq_open(qname, O_CREAT | O_WRONLY,
				  S_IRUSR | S_IWUSR, &attr);
		if (woqueue == (mqd_t) - 1) {
			perror("mq_open() did not return success");
			printf("Test UNRESOLVED\n");
			/* kill child and exit */
			kill(pid, SIGABRT);
			return PTS_UNRESOLVED;
		}
#ifdef DEBUG
		printf("write-only message queue opened in parent\n");
#endif

		if (mq_send(woqueue, msgptr, strlen(msgptr), 1) != 0) {
			perror("mq_send() did not return success");
			printf("Test FAILED\n");
			/* kill child, close queue and exit */
			kill(pid, SIGABRT);
			mq_close(woqueue);
			mq_unlink(qname);
			return PTS_FAIL;
		}
#ifdef DEBUG
		printf("Message %s sent\n", msgptr);
#endif

		if (mq_receive(woqueue, msgrcd, BUFFER, &pri) != -1) {
			printf("mq_receive() ret success w write only queue\n");
			printf("Test FAILED\n");
			/* kill child, close queue and exit */
			kill(pid, SIGABRT);
			mq_close(woqueue);
			mq_unlink(qname);
			return PTS_FAIL;
		}
#ifdef DEBUG
		printf("Message receive failed, as expected\n");
#endif

		sleep(1);
		kill(pid, SIGUSR1);

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit");
			printf("Test UNRESOLVED\n");
			/* close queue and exit */
			mq_close(woqueue);
			mq_unlink(qname);
			return PTS_UNRESOLVED;
		}
#ifdef DEBUG
		printf("Child finished\n");
#endif

		mq_close(woqueue);
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
