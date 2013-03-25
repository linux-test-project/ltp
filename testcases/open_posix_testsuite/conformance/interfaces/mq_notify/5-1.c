/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_notify() test plan:
 *  If a process has registered for notification of message arrival
 *  at a message queue, while some thread is blocked in mq_receive
 *  waiting to receive a message, the arriving message should satisfy
 *  the appropriate mq_receive(). In this case, no notification will
 *  be sent.
 *  NOTE:  The test makes some assumptions and has some potential race
 *  conditions, but seems the best way to test for now.
 *
 *  3/28/2003	Fix a bug mentioned by Michal Wronski, pass mq_attr struct
 *  		to the mq_open().
 *
 *  4/11/2003 	change sa_flags from SA_RESTART to 0 to avoid compile
 *  		error.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "mq_notify"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE	50
#define MSG_SIZE	40
#define BUFFER 40

int enter_handler = 0;

void msg_handler()
{
	enter_handler = 1;
}

void mqclean(mqd_t queue, const char *qname)
{
	mq_close(queue);
	mq_unlink(qname);
}

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes;
	const char s_msg_ptr[MSG_SIZE] = "test message \n";
	char r_msg_ptr[MSG_SIZE];
	struct sigevent notification;
	struct sigaction sa;
	unsigned int prio = 1;
	int pid;
	struct mq_attr attr;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());
	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		mqclean(mqdes, mqname);
		return PTS_UNRESOLVED;
	}

	pid = fork();
	if (pid == -1) {
		perror(ERROR_PREFIX "fork");
		mqclean(mqdes, mqname);
		return PTS_UNRESOLVED;
	}
	if (pid == 0) {
		/* child process */
		mq_receive(mqdes, r_msg_ptr, MSG_SIZE, NULL);
		return 0;
	} else {
		/* parent process */
		sleep(2);	/* after 2 seconds,
				   assume that child with block on mq_receive. */
		notification.sigev_notify = SIGEV_SIGNAL;
		notification.sigev_signo = SIGUSR1;
		sa.sa_handler = msg_handler;
		sa.sa_flags = 0;
		sigaction(SIGUSR1, &sa, NULL);
		if (mq_notify(mqdes, &notification) != 0) {
			perror(ERROR_PREFIX "mq_notify");
			return PTS_UNRESOLVED;
		}
		if (mq_send(mqdes, s_msg_ptr, MSG_SIZE, prio) == -1) {
			perror(ERROR_PREFIX "mq_send");
			return PTS_UNRESOLVED;
		}
		sleep(1);
		if (mq_unlink(mqname) != 0) {
			perror(ERROR_PREFIX "mq_unlink");
			return PTS_UNRESOLVED;
		}
		if (enter_handler) {
			printf("Test FAILED \n");
			return PTS_FAIL;
		}
		printf("Test PASSED \n");
		mqclean(mqdes, mqname);
		return PTS_PASS;
	}
}
