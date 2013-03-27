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

	sprintf(qname, "/mq_open_7-2_%d", getpid());

	pid = fork();
	if (pid == 0) {
		mqd_t roqueue;
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

		/* wait for parent to finish mq_send */
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		sigwait(&mask, &sig);

		/* once parent has finished sending, open read-only queue */
		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		roqueue = mq_open(qname, O_RDONLY, S_IRUSR | S_IWUSR, &attr);
		if (roqueue == (mqd_t) - 1) {
			perror("mq_open() read only failed");
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Readonly message queue opened in child\n");
#endif

		if (mq_receive(roqueue, msgrcd, BUFFER, &pri) == -1) {
			perror("mq_receive() on a read only queue failed");
			mq_close(roqueue);
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Received message %s\n", msgrcd);
#endif

		if (mq_send(roqueue, msgptr, strlen(msgptr) + 1, 1) == 0) {
			printf("mq_send() on a read only queue succeeded\n");
			mq_close(roqueue);
			return CHILDFAIL;
		}
#ifdef DEBUG
		printf("Sending message failed in child, as expected\n");
#endif
		mq_close(roqueue);

		return CHILDPASS;
	} else {
		/* parent here */
		mqd_t rdwrqueue;
		struct mq_attr attr;
		int i;

		attr.mq_msgsize = BUFFER;
		attr.mq_maxmsg = BUFFER;
		rdwrqueue = mq_open(qname, O_CREAT | O_RDWR,
				    S_IRUSR | S_IWUSR, &attr);
		if (rdwrqueue == (mqd_t) - 1) {
			perror("mq_open() did not return success");
			printf("Test UNRESOLVED\n");
			/* kill child and exit */
			kill(pid, SIGABRT);
			return PTS_UNRESOLVED;
		}
#ifdef DEBUG
		printf("Message queue opened in parent\n");
#endif

		if (mq_send(rdwrqueue, msgptr, strlen(msgptr) + 1, 1) != 0) {
			perror("mq_send() did not return success");
			printf("Test UNRESOLVED\n");
			/* close queue, kill child and exit */
			mq_close(rdwrqueue);
			mq_unlink(qname);
			kill(pid, SIGABRT);
			return PTS_UNRESOLVED;
		}
#ifdef DEBUG
		printf("Message %s sent\n", msgptr);
#endif

		sleep(1);
		kill(pid, SIGUSR1);

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit");
			printf("Test UNRESOLVED\n");
			/* close queue and exit */
			mq_close(rdwrqueue);
			mq_unlink(qname);
			return PTS_UNRESOLVED;
		}
#ifdef DEBUG
		printf("Child finished\n");
#endif

		mq_close(rdwrqueue);
		mq_unlink(qname);

		if (!WIFEXITED(i) || !WEXITSTATUS(i)) {
			printf("Test FAILED: exit status %d\n", i);
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}
