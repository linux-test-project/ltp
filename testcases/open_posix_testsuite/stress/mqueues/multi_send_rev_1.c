
/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test whether message queue can work correctly under lots of usage.
 * 1. Many threads sending/receiving on different message queue.
 * 2. Set different Priority to the messages in the message queue, to see whether the highest priority is received first.
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

#define MSG_SIZE	128
#define MAX_MSG		3
#define Max_Threads	10
#define Name_Size	20

typedef struct {
	int ThreadID;
	mqd_t mqID;
} mq_info;

int *send(void *info)
{
	int i;

	const char *s_msg_ptr[] = { "msg test 1", "msg test 2", "msg test 3" };
	mq_info send_info;
	send_info.ThreadID = ((mq_info *) info)->ThreadID;
	send_info.mqID = ((mq_info *) info)->mqID;
	printf("Enter into send [%d], mq = %d \n", send_info.ThreadID,
	       send_info.mqID);
	for (i = 0; i < MAX_MSG; i++) {
		if (-1 == mq_send(send_info.mqID, s_msg_ptr[i], MSG_SIZE, i)) {
			perror("mq_send doesn't return success \n");
			pthread_exit((void *)1);
		}
		printf("[%d] send '%s' in thread send [%d]. \n", i + 1,
		       s_msg_ptr[i], send_info.ThreadID);
	}
	pthread_exit(NULL);

}

int *receive(void *info)
{
	int i;
	char r_msg_ptr[MAX_MSG][MSG_SIZE];

	mq_info recv_info;
	recv_info.ThreadID = ((mq_info *) info)->ThreadID;
	recv_info.mqID = ((mq_info *) info)->mqID;
	printf("Enter into receive [%d], mq = %d \n", recv_info.ThreadID,
	       recv_info.mqID);
	for (i = 0; i < MAX_MSG; i++) {
		if (-1 ==
		    mq_receive(recv_info.mqID, r_msg_ptr[i], MSG_SIZE, NULL)) {
			perror("mq_receive doesn't return success \n");
			pthread_exit(NULL);
		}
		printf("[%d] receive '%s' in thread receive recv [%d]. \n",
		       i + 1, r_msg_ptr[i], recv_info.ThreadID);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	const char *MQ_NAME[Max_Threads] =
	    { "/msg1", "/msg2", "/msg3", "/msg4", "/msg5", "/msg6", "/msg7",
"/msg8", "/msg9", "/msg10" };
	mqd_t mq[Max_Threads];
	struct mq_attr mqstat;
	int oflag = O_CREAT | O_NONBLOCK | O_RDWR;
	int num, i;
	pthread_t sed[Max_Threads], rev[Max_Threads];
	mq_info info[Max_Threads];

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

	for (i = 0; i < num; i++) {
		if ((mq[i] = mq_open(MQ_NAME[i], oflag, 0777, &mqstat)) == (mqd_t)-1) {
			perror("mq_open doesn't return success \n");
			return PTS_UNRESOLVED;
		}
		printf("mq[%i] created \n", i);
	}
	for (i = 0; i < num; i++) {
		info[i].ThreadID = i;
		info[i].mqID = mq[i];
		pthread_create(&sed[i], NULL, (void *)send, (void *)&info[i]);
		pthread_create(&rev[i], NULL, (void *)receive,
			       (void *)&info[i]);
	}
	for (i = 0; i < num; i++) {
		pthread_join(sed[i], NULL);
		pthread_join(rev[i], NULL);
	}

	for (i = 0; i < num; i++) {
		mq_close(mq[i]);
		mq_close(mq[i]);
		mq_unlink(MQ_NAME[i]);
		mq_unlink(MQ_NAME[i]);
	}
	return PTS_PASS;
}
