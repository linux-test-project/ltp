/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test mq_timedsend() will set errno == EINTR if it is interrupted by a signal.
 *
 * Steps:
 * 1. Create a thread and set up a signal handler for SIGUSR1
 * 2. Thread indicates to main that it is ready to start calling mq_timedsend
 *    until it blocks for a timeout of 10 seconds.
 * 3. In main, send the thread the SIGUSR1 signal while mq_timedsend is
 *    blocking.
 * 4. Check to make sure that mq_timedsend blocked, and that it returned
 *    EINTR when it was interrupted by SIGUSR1.
 */

#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 10
#define TIMEOUT 10		/* seconds mq_timedsend will block */
#define SIGNAL_DELAY_MS 50	/* delay in ms between 2 signals */

#define error_and_exit(en, msg) \
	do { errno = en; perror(msg); exit(PTS_UNRESOLVED); } while (0)

/* variable to indicate how many times signal handler was called */
static volatile int in_handler;

/* errno returned by mq_timedsend() */
static int mq_timedsend_errno = -1;

pthread_barrier_t barrier;

/*
 * This handler is just used to catch the signal and stop sleep (so the
 * parent knows the child is still busy sending signals).
 */
void justreturn_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	in_handler++;
}

void *a_thread_func(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int i, ret;
	struct sigaction act;
	char gqname[NAMESIZE];
	mqd_t gqueue;
	const char *msgptr = MSGSTR;
	struct mq_attr attr;
	struct timespec ts;

	/* Set up handler for SIGUSR1 */
	act.sa_handler = justreturn_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	/* Set up mq */
	sprintf(gqname, "/mq_timedsend_12-1_%d", getpid());

	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = BUFFER;
	gqueue = mq_open(gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (gqueue == (mqd_t) -1)
		error_and_exit(errno, "mq_open");

	/* mq_timedsend will block for TIMEOUT seconds when it waits */
	ts.tv_sec = time(NULL) + TIMEOUT;
	ts.tv_nsec = 0;

	/* main can now start sending SIGUSR1 signal */
	ret = pthread_barrier_wait(&barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
		error_and_exit(ret, "pthread_barrier_wait start");

	for (i = 0; i < MAXMSG + 1; i++) {
		ret = mq_timedsend(gqueue, msgptr, strlen(msgptr), 1, &ts);
		if (ret == -1) {
			mq_timedsend_errno = errno;
			break;
		}
	}

	if (mq_unlink(gqname) != 0)
		error_and_exit(errno, "mq_unlink");

	switch (mq_timedsend_errno) {
	case -1:
		mq_timedsend_errno = 0;
		printf("Error: mq_timedsend wasn't interrupted\n");
		break;
	case EINTR:
		printf("thread: mq_timedsend interrupted by signal"
			" and correctly set errno to EINTR\n");
		break;
	default:
		printf("mq_timedsend not interrupted by signal or"
			" set errno to incorrect code: %d\n",
			mq_timedsend_errno);
		break;
	}

	/* wait until main stops sending signals */
	ret = pthread_barrier_wait(&barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
		error_and_exit(ret, "pthread_barrier_wait end");
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t new_th;
	int i = 0, ret;

	ret = pthread_barrier_init(&barrier, NULL, 2);
	if (ret != 0)
		error_and_exit(ret, "pthread_barrier_init");

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret != 0)
		error_and_exit(ret, "pthread_create");

	/* wait for thread to start */
	ret = pthread_barrier_wait(&barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
		error_and_exit(ret, "pthread_barrier_wait start");

	struct timespec completion_wait_ts = {0, SIGNAL_DELAY_MS*1000000};
	while (i < TIMEOUT*1000 && mq_timedsend_errno < 0) {
		/* signal thread while it's in mq_timedsend */
		ret = pthread_kill(new_th, SIGUSR1);
		if (ret != 0)
			error_and_exit(ret, "pthread_kill");
		nanosleep(&completion_wait_ts, NULL);
		i += SIGNAL_DELAY_MS;
	}

	/* thread can now safely exit */
	ret = pthread_barrier_wait(&barrier);
	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
		error_and_exit(ret, "pthread_barrier_wait end");

	ret = pthread_join(new_th, NULL);
	if (ret != 0)
		error_and_exit(ret, "pthread_join");

	/* Test to see if the thread blocked correctly in mq_timedsend,
	 * and if it returned EINTR when it caught the signal */
	if (mq_timedsend_errno != EINTR) {
		printf("Error: mq_timedsend was NOT interrupted\n");
		printf(" signal handler was called %d times\n", in_handler);
		printf(" SIGUSR1 signals sent: %d\n", i);
		printf(" last mq_timedsend errno: %d %s\n",
			mq_timedsend_errno, strerror(mq_timedsend_errno));
		if (in_handler == 0) {
			printf("Error: SIGUSR1 was never received\n");
			return PTS_UNRESOLVED;
		}
		return PTS_FAIL;
	}

	ret = pthread_barrier_destroy(&barrier);
	if (ret != 0)
		error_and_exit(ret, "pthread_barrier_destroy");

	printf("Test PASSED\n");
	return PTS_PASS;
}
