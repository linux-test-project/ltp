/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_notify() test plan:
 *  If the notification is NULL and the process is currently registered
 *  for notification by the specified message queue, the existing registration
 *  will be removed.
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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "mq_notify"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define MSG_SIZE	50
#define NAMESIZE	50

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
	struct sigevent notification;
	struct sigaction sa;
	unsigned int prio = 1;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}
	notification.sigev_notify = SIGEV_SIGNAL;
	notification.sigev_signo = SIGUSR1;
	sa.sa_handler = msg_handler;
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
	if (mq_notify(mqdes, &notification) != 0) {
		perror(ERROR_PREFIX "mq_notify");
		mqclean(mqdes, mqname);
		return PTS_UNRESOLVED;
	}
	if (mq_notify(mqdes, NULL) != 0) {
		printf("Test FAILED \n");
		mqclean(mqdes, mqname);
		return PTS_FAIL;
	}
	if (mq_send(mqdes, s_msg_ptr, MSG_SIZE, prio) == -1) {
		perror(ERROR_PREFIX "mq_send");
		mqclean(mqdes, mqname);
		return PTS_UNRESOLVED;
	}
	sleep(1);
	if (mq_unlink(mqname) != 0) {
		perror(ERROR_PREFIX "mq_unlink");
		return PTS_UNRESOLVED;
	}
	if (!enter_handler) {
		printf("Test PASSED \n");
		mqclean(mqdes, mqname);
		return PTS_PASS;
	} else {
		printf("Test FAILED \n");
		mqclean(mqdes, mqname);
		return PTS_FAIL;
	}
}
