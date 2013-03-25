/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_notify() test plan:
 *  Only one process may be registerd for notification by a message queue.
 *  If the calling process or other process has already registered for the
 *  notification, subsequent attempts to register for that message queue
 *  will fail.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "mq_notify"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50

void mqclean(mqd_t queue, const char *qname)
{
	mq_close(queue);
	mq_unlink(qname);
}

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes;
	struct sigevent notification;
	int pid;
	int status;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}

	notification.sigev_notify = SIGEV_SIGNAL;
	notification.sigev_signo = SIGUSR1;
	if (mq_notify(mqdes, &notification) != 0) {
		printf("Test FAILED \n");
		mqclean(mqdes, mqname);
		return PTS_FAIL;
	}
	pid = fork();
	if (pid == -1) {
		perror(ERROR_PREFIX "fork");
		mqclean(mqdes, mqname);
		return PTS_UNRESOLVED;
	}
	if (pid == 0) {
		/* child process */
		if (mq_notify(mqdes, &notification) != -1) {
			printf("Test FAILED \n");
			return PTS_FAIL;
		} else {
			printf("Test PASSED \n");
			return PTS_PASS;
		}
	} else {
		/* parent process */
		wait(&status);
		mqclean(mqdes, mqname);
		return status;
	}
}
