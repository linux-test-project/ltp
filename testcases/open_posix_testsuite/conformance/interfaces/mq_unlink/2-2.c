/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_unlink() test plan:
 *  If one or more process have the message queue open, destruction of the
 *  message queue will be postponed until all reference to the message queue
 *  have been closed. At this time, call to mq_open() with O_CREAT flag may fail
 *  until the message queue is actually removed.
 *  Steps:
 *  1. Create 2 pipes to communicate with parent and child processes.
 *  2. Parent uses mq_open to create a new mq and tell child to open it using pipe.
 *  3. Child open the mq and tell Parent, so mq has 2 reference now.
 *  4. Parent mq_unlink the mq and tell Child to close this mq.
 *  5. Child close the mq and tell parent.
 *  6. Parent recreate the mq using mq_open, if the mq is actually removed, this operation
 *     will succeed, if not, it may fail.
 */

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define PIPE_READ  0
#define PIPE_WRITE 1

#define TEST "2-2"
#define FUNCTION "mq_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int parent_process(char *mqname, int read_pipe, int write_pipe, pid_t child_pid);
int child_process(char *mqname, int read_pipe, int write_pipe);
int send_receive(int read_pipe, int write_pipe, char send, char *reply);

int main(void)
{
	char mqname[50];
	pid_t pid;
	int to_parent[2];
	int to_child[2];
	int rval;
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());
	rval = pipe(to_parent);
	if (rval == -1) {
		perror(ERROR_PREFIX "fd[0]");
		return PTS_UNRESOLVED;
	}
	rval = pipe(to_child);
	if (rval == -1) {
		perror(ERROR_PREFIX "fd[1]");
		return PTS_UNRESOLVED;
	}
	pid = fork();
	if (pid == -1) {
		perror(ERROR_PREFIX "fork");
		return PTS_UNRESOLVED;
	}
	if (pid == 0) {
		//child process
		close(to_parent[PIPE_READ]);
		close(to_child[PIPE_WRITE]);
		return child_process(mqname, to_child[PIPE_READ],
				     to_parent[PIPE_WRITE]);
	} else {
		//parent process
		close(to_parent[PIPE_WRITE]);
		close(to_child[PIPE_READ]);
		return parent_process(mqname, to_parent[PIPE_READ],
				      to_child[PIPE_WRITE], pid);
	}
}

int parent_process(char *mqname, int read_pipe, int write_pipe,
	pid_t child_pid LTP_ATTRIBUTE_UNUSED)
{
	mqd_t mqdes;
	char reply;
	int rval;

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t)-1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}
	// Tell child a message queue has been opened.
	rval = send_receive(read_pipe, write_pipe, 'a', &reply);
	if (rval) {
		return rval;
	}
	if (reply != 'b') {
		printf(ERROR_PREFIX "send_receive: " "expected a 'b'\n");
		return PTS_UNRESOLVED;
	}
	if (mq_unlink(mqname) == 0) {
		rval = send_receive(read_pipe, write_pipe, 'c', &reply);
		if (rval) {
			return rval;
		}
		if (reply != 'd') {
			printf(ERROR_PREFIX "send_receive: "
			       "expected a 'd'\n");
			return PTS_UNRESOLVED;
		}
		if (mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0) !=
		    (mqd_t)-1) {
			if (mq_unlink(mqname) != 0) {
				perror(ERROR_PREFIX "mq_unlink(2)");
				return PTS_UNRESOLVED;
			}
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf
			    ("mq_open may fail until the message queue is actually removed \n");
			printf("Test PASSED\n");
			return PTS_PASS;
		}
	}
	printf(ERROR_PREFIX "mq_unlink\n");
	return PTS_UNRESOLVED;
}

int child_process(char *mqname, int read_pipe, int write_pipe)
{
	mqd_t mqdes;
	int rval;
	char reply;

	rval = send_receive(read_pipe, write_pipe, 0, &reply);
	if (rval) {
		return rval;
	}
	if (reply != 'a') {
		printf(ERROR_PREFIX "send_receive: " "expected an 'a'");
		return PTS_UNRESOLVED;
	}
	mqdes = mq_open(mqname, O_RDWR, 0, 0);
	if (mqdes == (mqd_t)-1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}
	rval = send_receive(read_pipe, write_pipe, 'b', &reply);
	if (rval) {
		return rval;
	}
	if (reply != 'c') {
		printf(ERROR_PREFIX "send_receive: " "expected a 'c'\n");
		return PTS_UNRESOLVED;
	}
	if (mq_close(mqdes) == -1) {
		perror(ERROR_PREFIX "mq_close");
		return PTS_UNRESOLVED;
	}
	rval = send_receive(read_pipe, write_pipe, 'd', NULL);
	if (rval) {
		return rval;
	}
	return 0;
}

int send_receive(int read_pipe, int write_pipe, char send, char *reply)
{
	ssize_t bytes;
	if (send) {
		bytes = write(write_pipe, &send, 1);
		if (bytes == -1) {
			perror(ERROR_PREFIX "write fd[1]");
			return PTS_UNRESOLVED;
		}
	}
	if (reply) {
		bytes = read(read_pipe, reply, 1);
		if (bytes == -1) {
			perror(ERROR_PREFIX "read fd[0]");
			return PTS_UNRESOLVED;
		} else if (bytes == 0) {
			printf(ERROR_PREFIX "read: EOF\n");
			return PTS_UNRESOLVED;
		}
	}
	return 0;
}
