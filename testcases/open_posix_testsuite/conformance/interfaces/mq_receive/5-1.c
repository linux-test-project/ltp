/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_receive() test plan:
 * Test that if the message queue is empty and O_NONBLOCK is not set,
 * mq_receive() will block until a message is enqueued on the message queue
 * or until mq_receive() is interrupted by a signal.
 *
 * NOTE:  This test makes some assumptions and has some potential race
 * conditions, but seems the best way to test for now.
 *
 * 3/28/2003	Fix a bug mentioned by Michal Wronski, pass mq_attr struct
 * 		to the mq_open().
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
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "mq_receive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40

int main(void)
{
	char mqname[NAMESIZE], msgrv[BUFFER];
	const char *msgptr = "test message ";
	mqd_t mqdes;
	int prio = 1;
	int pid;
	struct mq_attr attr;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	if ((pid = fork()) != 0) {
		/* Parent process */
		int status;
		if (mq_receive(mqdes, msgrv, BUFFER, NULL) > 0) {
			wait(&status);
			if (status) {
				printf("mq_send error \n");
				unresolved = 1;
			}
		} else {
			wait(NULL);	/* wait for child to exit */
			perror(ERROR_PREFIX "mq_receive");
			failure = 1;
		}
		if (mq_close(mqdes) != 0) {
			perror(ERROR_PREFIX "mq_close");
			unresolved = 1;
		}
		if (mq_unlink(mqname) != 0) {
			perror(ERROR_PREFIX "mq_unlink");
			unresolved = 1;
		}
		if (failure == 1) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		if (unresolved == 1) {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		}
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		/*  Child Process */
		sleep(2);	/* sleep 2 seconds,
				   assume that child will block on waiting then */
		if (mq_send(mqdes, msgptr, strlen(msgptr), prio) == -1) {
			perror(ERROR_PREFIX "mq_send");
			return PTS_UNRESOLVED;
		}
		return 0;
	}
	return PTS_UNRESOLVED;
}
