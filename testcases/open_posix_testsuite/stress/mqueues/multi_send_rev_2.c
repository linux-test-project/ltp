/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test whether message queue can work correctly under lots of usage.
 * 1. Many threads sending/receiving on the same message queue.
 * 2. Set different Priority to the messages in the message queue, to see
 * whether the highest priority is received first.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <limits.h>
#include <mqueue.h>

#include "posixtest.h"

#define MQ_NAME       "/testmsg1"
#define MSG_SIZE	128
#define MAX_MSG		5
#define Max_Threads	100

const char *s_msg_ptr[] =
    { "send_1 1", "send_1 2", "send_1 3", "send_1 4", "send_1 5" };
char r_msg_ptr[Max_Threads][MAX_MSG][MSG_SIZE];
mqd_t mq = 0;

int *send(void *ID)
{
	int i;
	int ThreadID = *(int *)ID;

	printf("Enter into send [%d] \n", ThreadID);
	for (i = 0; i < MAX_MSG; i++) {
		if (-1 == mq_send(mq, s_msg_ptr[i], MSG_SIZE, i)) {
			perror("mq_send doesn't return success \n");
			pthread_exit((void *)1);
		}
		printf("[%d] send '%s' in thread send %d. \n", i + 1,
		       s_msg_ptr[i], ThreadID);
	}
	pthread_exit(NULL);

}

int *receive(void *ID)
{
	int i;
	int ThreadID = *(int *)ID;

	printf("Enter into receive[%d] \n", ThreadID);
	for (i = 0; i < MAX_MSG; i++) {
		if (-1 ==
		    mq_receive(mq, r_msg_ptr[ThreadID][i], MSG_SIZE, NULL)) {
			perror("mq_receive doesn't return success \n");
			pthread_exit((void *)1);
		}
		printf("[%d] receive '%s' in thread receive[%d]. \n", i + 1,
		       r_msg_ptr[ThreadID][i], ThreadID);
	}
	printf("receive[%d] quit ...\n", ThreadID);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	struct mq_attr mqstat;
	int oflag = O_CREAT | O_NONBLOCK | O_RDWR;
	pthread_t sed[Max_Threads], rev[Max_Threads];
	int ThreadID[Max_Threads];
	int num, i;

/* #ifndef  _POSIX_MESSAGE_PASSING
	printf("_POSIX_MESSAGE_PASSING is not defined \n");
	return PTS_UNRESOLVED;
#endif */
	if ((2 != argc) || ((num = atoi(argv[1])) <= 0)) {
		fprintf(stderr, "Usage: %s number_of_threads\n", argv[0]);
		return PTS_FAIL;
	}
	if (num > Max_Threads) {
		printf("The num of threads are too large.  Reset to %d\n",
		       Max_Threads);
		num = Max_Threads;
	}
	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_maxmsg = MAX_MSG;
	mqstat.mq_msgsize = MSG_SIZE;
	mqstat.mq_flags = 0;

	if ((mq = mq_open(MQ_NAME, oflag, 0777, &mqstat)) == (mqd_t)-1) {
		printf("mq_open doesn't return success\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < num; i++) {
		ThreadID[i] = i;
		pthread_create(&sed[i], NULL, (void *)send,
			       (void *)&ThreadID[i]);
		pthread_create(&rev[i], NULL, (void *)receive,
			       (void *)&ThreadID[i]);
	}

	for (i = 0; i < num; i++) {
		pthread_join(sed[i], NULL);
		pthread_join(rev[i], NULL);
	}
	mq_close(mq);
	mq_unlink(MQ_NAME);
	return PTS_PASS;
}
