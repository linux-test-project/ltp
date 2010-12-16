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
 * 2. Thread indicates to main that it is ready to start calling mq_timedsend until it
 *    blocks for a timeout of 10 seconds.
 * 3. In main, send the thread the SIGUSR1 signal while mq_timedsend is blocking.
 * 4. Check to make sure that mq_timedsend blocked, and that it returned EINTR when it was
 *    interrupted by SIGUSR1.
 *
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

#define INTHREAD 0
#define INMAIN 1

int sem; 		/* manual semaphore */
int in_handler;		/* flag to indicate signal handler was called */
int errno_eintr;	/* flag to indicate that errno was set to eintr when mq_timedsend()
			   was interruped. */

/*
 * This handler is just used to catch the signal and stop sleep (so the
 * parent knows the child is still busy sending signals).
 */
void justreturn_handler(int signo)
{
	/* Indicate that the signal handler was called */
	in_handler=1;
	return;
}

void *a_thread_func()
{

		int i;
		struct sigaction act;
		char gqname[NAMESIZE];
		mqd_t gqueue;
		const char *msgptr = MSGSTR;
		struct mq_attr attr;
		struct timespec ts;

		/* Set up handler for SIGUSR1 */
		act.sa_handler=justreturn_handler;
		act.sa_flags=0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGUSR1, &act, 0);

		/* Set up mq */
        	sprintf(gqname, "/mq_timedsend_12-1_%d", getpid());

		attr.mq_maxmsg = MAXMSG;
		attr.mq_msgsize = BUFFER;
       		gqueue = mq_open(gqname, O_CREAT |O_RDWR, S_IRUSR | S_IWUSR, &attr);
        	if (gqueue == (mqd_t)-1) {
                	perror("mq_open() did not return success");
			pthread_exit((void*)PTS_UNRESOLVED);
                	return NULL;
        	}

		/* mq_timedsend will block for 10 seconds when it waits */
		ts.tv_sec=time(NULL)+10;
		ts.tv_nsec=0;

		/* Tell main it can go ahead and start sending SIGUSR1 signal */
		sem = INMAIN;

		for (i=0; i<MAXMSG+1; i++) {
        		if (mq_timedsend(gqueue, msgptr,
					strlen(msgptr), 1, &ts) == -1) {
				if (errno == EINTR) {
					if (mq_unlink(gqname) != 0) {
                				perror("mq_unlink() did not return success");
						pthread_exit((void*)PTS_UNRESOLVED);
                				return NULL;
					}
					printf("thread: mq_timedsend interrupted by signal and correctly set errno to EINTR\n");
					errno_eintr=1;
					pthread_exit((void*)PTS_PASS);
					return NULL;
				} else {
				printf("mq_timedsend not interrupted by signal or set errno to incorrect code: %d\n", errno);
					pthread_exit((void*)PTS_FAIL);
					return NULL;
				}
        		}
		}

		/* Tell main that it the thread did not block like it should have */
		sem = INTHREAD;

		perror("Error: thread never blocked\n");
		pthread_exit((void*)PTS_FAIL);
		return NULL;
}

int main()
{
        pthread_t new_th;
	int i;

	/* Initialized values */
	i = 0;
	in_handler=0;
	errno_eintr=0;
	sem = INTHREAD;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{
		perror("Error: in pthread_create\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to set up handler for SIGUSR1 */
	while (sem==INTHREAD)
	 	sleep(1);

	while ((i != 10) && (sem==INMAIN))
	{
		/* signal thread while it's in mq_timedsend */
		if (pthread_kill(new_th, SIGUSR1) != 0)
		{
			perror("Error: in pthread_kill\n");
			return PTS_UNRESOLVED;
		}
		i++;
	}

	if (pthread_join(new_th, NULL) != 0)
	{
		perror("Error: in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Test to see if the thread blocked correctly in mq_timedsend, and if it returned
	 * EINTR when it caught the signal */
	if (errno_eintr != 1)
	{
		if (sem==INTHREAD)
		{
			printf("Test FAILED: mq_timedsend() never blocked for any timeout period.\n");
			return PTS_FAIL;
		}

		if (in_handler != 0)
		{
			perror("Error: signal SIGUSR1 was never received, and/or the signal handler was never called.\n");
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}