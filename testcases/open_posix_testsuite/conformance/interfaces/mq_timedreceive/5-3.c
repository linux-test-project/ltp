/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_timedreceive() test plan:
 * Test that if the message queue is empty and O_NONBLOCK is not set,
 * mq_timedreceive() will block until mq_timedreceive() is interrupted by a
 * signal.
 *
 * NOTE:  This test makes some assumptions and has some potential race
 * conditions, but seems the best way to test for now.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "5-3"
#define FUNCTION "mq_timedreceive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40

void stopreceive(int signo LTP_ATTRIBUTE_UNUSED)
{
	return;
}

int main(void)
{
	char mqname[NAMESIZE], msgrv[BUFFER];
	mqd_t mqdes;
	int pid;
	struct mq_attr attr;
	struct timespec ts;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	pid = fork();
	if (pid != 0) {
		/* Parent process */
		struct sigaction act;
		act.sa_handler = stopreceive;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGABRT, &act, 0);

		ts.tv_sec = INT32_MAX;
		ts.tv_nsec = 0;

		if (mq_timedreceive(mqdes, msgrv, BUFFER, NULL, &ts) == -1) {
			wait(NULL);
			if (EINTR != errno) {
				printf("errno != EINTR\n");
				failure = 1;
			}
		} else {
			wait(NULL);
			printf("mq_timedreceive() succeed unexpectly\n");
			failure = 1;
		}
		if (mq_close(mqdes) != 0) {
			perror("mq_close() did not return success");
			unresolved = 1;
		}
		if (mq_unlink(mqname) != 0) {
			perror("mq_unlink() did not return success");
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
		sleep(1);	/* give time to parent to set up handler */
		/* send signal to parent */
		kill(getppid(), SIGABRT);
	}
}
