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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <mqueue.h>

#include "posixtest.h"

#define MQ_NAME       "/testmsg"
#define MSG_SIZE	128
#define MAX_MSG		3

int main(int argc, char *argv[])
{
	mqd_t mq = 0;
	pid_t pid;
	struct mq_attr mqstat, attr;
	int oflag = O_CREAT|O_RDWR;
	const char *s_msg_ptr[] = {"msg test 1", "msg test 2", "msg test 3"};
	char r_msg_ptr[MAX_MSG][MSG_SIZE];
	int i;

	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_maxmsg = MAX_MSG;
	mqstat.mq_msgsize = MSG_SIZE;
	mqstat.mq_flags = 0;

/* #ifndef  _POSIX_MESSAGE_PASSING
	printf("_POSIX_MESSAGE_PASSING is not defined \n");
	return PTS_UNRESOLVED;
#endif */ 
  
  	if( ((mqd_t) -1) == (mq = mq_open(MQ_NAME,oflag,0777, &mqstat)) ) {
		perror("mq_open doesn't return success \n");
		return PTS_UNRESOLVED;
	}
	
	if ( 0 != (pid = fork() )) {
		mq_getattr(mq, &attr);
		for (i=0; i < MAX_MSG; i++) {
			printf("[%d] s_msg_ptr is '%s' \n", i+1, s_msg_ptr[i]);
			printf("Prepare to send message...\n");
			if ( -1 == mq_send(mq, s_msg_ptr[i], attr.mq_msgsize, 1)) {
				perror("mq_send doesn't return success \n");
				mq_close(mq);
				mq_unlink(MQ_NAME);
				return PTS_UNRESOLVED;
			}
			printf("Process %ld send message '%s' to process %ld \n", (long)getpid(), s_msg_ptr[i], (long)pid); 
		}
		wait(NULL);
	}
	else {
		printf("Enter into child process...\n");
		mq_getattr(mq, &attr);
		for (i = 0; i < MAX_MSG; i++) {
			printf("Prepare to receive [%d] messages...\n", i+1);
			if (-1 == mq_receive(mq, r_msg_ptr[i], attr.mq_msgsize, NULL)) {
				perror("mq_receive doesn't return success \n");
				mq_close(mq);
				mq_unlink(MQ_NAME);
				return PTS_UNRESOLVED;
			}
			printf("process %ld receive message '%s' from process %ld \n", (long)getpid(), r_msg_ptr[i], (long)getppid());
		}
	}
		
	mq_close(mq);
	mq_unlink(MQ_NAME);
	return PTS_PASS;
}

