/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test two processes can read/write from the same message queue
 * at the same time.
 *
 */

#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define MQ_NAME		"/testmsg"
#define MSG_SIZE	128
#define MAX_MSG		3

int main(void)
{
	struct mq_attr mqstat, attr;
	char r_msg_ptr[MAX_MSG][MSG_SIZE];
	const char *s_msg_ptr[] = { "msg test 1", "msg test 2", "msg test 3" };
	int i;
	int oflag = O_CREAT | O_RDWR;
	int ret_code = PTS_PASS;
	mqd_t mq = 0;
	pid_t pid;

	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_maxmsg = MAX_MSG;
	mqstat.mq_msgsize = MSG_SIZE;
	mqstat.mq_flags = 0;

/* #ifndef  _POSIX_MESSAGE_PASSING
	printf("_POSIX_MESSAGE_PASSING is not defined \n");
	return PTS_UNRESOLVED;
#endif */

	if (((mqd_t) - 1) == (mq = mq_open(MQ_NAME, oflag, 0777, &mqstat))) {
		perror("mq_open doesn't return success \n");
		return PTS_UNRESOLVED;
	}

	switch ((pid = fork())) {
	case -1:
		perror("fork");
		ret_code = PTS_UNRESOLVED;
		break;
	case 0:
		printf("Enter into child process...\n");
		mq_getattr(mq, &attr);
		for (i = 0; i < MAX_MSG && ret_code == PTS_PASS; i++) {
			printf("Prepare to receive [%d] messages...\n", i + 1);
			if (-1 ==
			    mq_receive(mq, r_msg_ptr[i], attr.mq_msgsize,
				       NULL)) {
				perror("mq_receive doesn't return success \n");
				ret_code = PTS_UNRESOLVED;
			} else {
				printf("process %ld receive message '%s' from "
				       "process %ld \n",
				       (long)getpid(), r_msg_ptr[i],
				       (long)getppid());
			}
		}
		exit(ret_code);
		break;
	default:
		mq_getattr(mq, &attr);
		for (i = 0; i < MAX_MSG && ret_code == PTS_PASS; i++) {
			printf("[%d] s_msg_ptr is '%s' \n", i + 1,
			       s_msg_ptr[i]);
			printf("Prepare to send message...\n");
			if (-1 == mq_send(mq, s_msg_ptr[i], strlen(s_msg_ptr[i]) + 1, 1)) {
				perror("mq_send doesn't return success \n");
				ret_code = PTS_UNRESOLVED;
			} else {
				printf("Process %ld send message '%s' to "
				       "process %ld \n",
				       (long)getpid(), s_msg_ptr[i], (long)pid);
			}
		}
		(void)wait(NULL);
		break;
	}

	(void)mq_close(mq);
	(void)mq_unlink(MQ_NAME);

	return ret_code;

}
