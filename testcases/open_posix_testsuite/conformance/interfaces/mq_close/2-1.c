/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
  mq_close test plan:
  1. Create pipes to communicate with child process
  2. Fork, child waits for command on pipe
  3. Create and open message queue, set up notification, sends command
  4. Child opens message queue and tries to set up notification, sends command
  5. Parent closes the queue, sends command
  6. Child tries again to set up notification (should succeed verifying that
      the close successfully removed notify association), sends command
  7. Parent reports result
*/

#include <signal.h>
#include <stdio.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "mq_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define PIPE_READ  0
#define PIPE_WRITE 1

int parent_process(char *qname, int read_pipe, int write_pipe, pid_t child_pid);
int child_process(char *qname, int read_pipe, int write_pipe);
mqd_t open_queue(char *qname, int oflag, int mode);
int send_receive(int read_pipe, int write_pipe, char send, char *reply);

int main(void)
{
	char qname[50];
	pid_t pid;
	int rval;
	int to_parent[2];
	int to_child[2];
	struct sigaction sa;

	sprintf(qname, "/" FUNCTION "_" TEST "_%d", getpid());

	rval = pipe(to_parent);
	if (rval == -1) {
		perror(ERROR_PREFIX "pipe (1)");
		return PTS_UNRESOLVED;
	}

	rval = pipe(to_child);
	if (rval == -1) {
		perror(ERROR_PREFIX "pipe (2)");
		return PTS_UNRESOLVED;
	}

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);

	pid = fork();
	if (pid == -1) {
		perror(ERROR_PREFIX "fork");
		return PTS_UNRESOLVED;
	}
	if (pid == 0) {
		// child process
		close(to_parent[PIPE_READ]);
		close(to_child[PIPE_WRITE]);

		return child_process(qname, to_child[PIPE_READ],
				     to_parent[PIPE_WRITE]);
	} else {
		// parent process
		close(to_parent[PIPE_WRITE]);
		close(to_child[PIPE_READ]);

		return parent_process(qname, to_parent[PIPE_READ],
				      to_child[PIPE_WRITE], pid);
	}

	return PTS_UNRESOLVED;
}

int parent_process(char *qname, int read_pipe, int write_pipe,
	pid_t child_pid LTP_ATTRIBUTE_UNUSED)
{
	mqd_t queue;
	struct sigevent se;
	int rval;
	char reply;

	queue = open_queue(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (queue == (mqd_t) - 1) {
		return PTS_UNRESOLVED;
	}

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = SIGUSR1;

	if (mq_notify(queue, &se)) {
		perror(ERROR_PREFIX "mq_notify (1)");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
	// send 'a' - signal child to verify it can't call notify
	rval = send_receive(read_pipe, write_pipe, 'a', &reply);
	if (rval) {
		mq_close(queue);
		mq_unlink(qname);
		return rval;
	}

	if (reply != 'b') {
		puts(ERROR_PREFIX "send_receive: " "expected a 'b'");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}
	// close the queue to perform test
	rval = mq_close(queue);
	mq_unlink(qname);
	if (rval) {
		perror(ERROR_PREFIX "mq_close:");
		return PTS_UNRESOLVED;
	}
	// send 'c' - signal child to verify it can call notify
	rval = send_receive(read_pipe, write_pipe, 'c', &reply);
	if (rval) {
		return rval;
	}

	if (reply == 'd') {
		puts("Test PASSED");
		return PTS_PASS;
	} else if (reply == 'e') {
		puts("Test FAILED");
		return PTS_FAIL;
	} else {
		puts(ERROR_PREFIX "bad reply from child");
		return PTS_UNRESOLVED;
	}
}

int child_process(char *qname, int read_pipe, int write_pipe)
{
	mqd_t queue;
	struct sigevent se;
	char reply;
	int rval;

	// wait for 'a' signal from parent
	rval = send_receive(read_pipe, write_pipe, 0, &reply);
	if (rval) {
		return rval;
	}

	if (reply != 'a') {
		puts(ERROR_PREFIX "send_receive: " "expected an 'a'");
		return PTS_UNRESOLVED;
	}
	// open the queue and attempt to set up notification
	queue = open_queue(qname, O_RDWR, 0);
	if (queue == (mqd_t) - 1) {
		return PTS_UNRESOLVED;
	}
	// try notify while parent still has queue open - should fail
	se.sigev_notify = SIGEV_SIGNAL;
	if (!mq_notify(queue, &se)) {
		puts(ERROR_PREFIX "mq_notify (2): " "should have failed");
		return PTS_UNRESOLVED;
	}
	// send 'b' - signal parent to close queue
	rval = send_receive(read_pipe, write_pipe, 'b', &reply);
	if (rval) {
		return rval;
	}

	if (reply != 'c') {
		puts(ERROR_PREFIX "send_receive: " "expected a 'c'");
		return PTS_UNRESOLVED;
	}
	// try notify after parent closed queue - should succeed
	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = 0;
	rval = mq_notify(queue, &se);

	// send 'd' for success and 'e' for failure
	send_receive(read_pipe, write_pipe, rval ? 'e' : 'd', NULL);

	return 0;
}

mqd_t open_queue(char *qname, int oflag, int mode)
{
	mqd_t queue;

	queue = mq_open(qname, oflag, mode, NULL);
	if (queue == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
	}

	return queue;
}

int send_receive(int read_pipe, int write_pipe, char send, char *reply)
{
	ssize_t bytes;

	if (send) {
		bytes = write(write_pipe, &send, 1);
		if (bytes == -1) {
			perror(ERROR_PREFIX "write (1)");
			return PTS_UNRESOLVED;
		}
	}

	if (reply) {
		bytes = read(read_pipe, reply, 1);
		if (bytes == -1) {
			perror(ERROR_PREFIX "read");
			return PTS_UNRESOLVED;
		} else if (bytes == 0) {
			puts(ERROR_PREFIX "read: EOF");
			return PTS_UNRESOLVED;
		}
	}

	return 0;
}
