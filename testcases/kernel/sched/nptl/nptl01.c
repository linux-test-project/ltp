/*
 * Copyright (c) International Business Machines  Corp., 2004.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : nptl01
 *
 *    EXECUTED BY       : root
 *
 *    TEST TITLE        : NPTL test for pthread_cond_timedwait() error
 *			  path bug.
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Neil Richards <neil_richards@uk.ibm.com>
 *
 *    DESCRIPTION
 *      This is a test for a bug found in the pthread_cond_timedwait() system call.
 *	of the Native POSIX Thread Library (NPTL) library code.
 *      There was an error path in the system call, where the sequence counters were
 *      getting updated w/o holding the internal condvar lock. A FAIL is indicated
 *	by the test hanging and not completing execution.
 *
 ****************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "test.h"

#define MAXTIME 72000		/* Maximum # of secs to wait before failing */
#define NUMLOOPS 100000		/* # of loops */

char *TCID = "nptl01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
void cleanup();

pthread_mutex_t req;
pthread_mutex_t ack;
pthread_mutex_t wait;
pthread_cond_t parent;
pthread_cond_t child;
int idle_count = 0;

/*
 * The time to wait should be set appropriately so that the waiting thread
 * is coming out of the wait at around the same time as the other thread is
 * signalling it.
 * The value of 1000 seems to work (ie. demonstrate the problem) on my
 * 8 way (hyperthreaded) 2GHz Xeon box.
 */
#define NSECS_TO_WAIT	(1)

void call_mutex_init(pthread_mutex_t * mutex, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_mutex_init(mutex, NULL)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_mutex_init failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void call_mutex_lock(pthread_mutex_t * mutex, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_mutex_lock(mutex)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_mutex_lock failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void call_mutex_unlock(pthread_mutex_t * mutex, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_mutex_unlock(mutex)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_mutex_unlock failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void call_cond_init(pthread_cond_t * cond, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_cond_init(cond, NULL)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_cond_init failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void call_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex,
		    char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_cond_wait(cond, mutex)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_cond_wait failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void call_cond_signal(pthread_cond_t * cond, char *buf, size_t buf_len)
{
	int ret;

	if ((ret = pthread_cond_signal(cond)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_cond_signal failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void do_timedwait(pthread_cond_t * cond, pthread_mutex_t * mutex,
		  char *buf, size_t buf_len, int i)
{
	struct timeval tv;
	struct timespec ts;
	int ret;

	if (gettimeofday(&tv, NULL) != 0) {
		tst_brkm(TBROK, cleanup, "gettimeofday failed: %s",
			 strerror_r(errno, buf, buf_len));
	}

	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = (tv.tv_usec * 1000) + NSECS_TO_WAIT;
	ts.tv_sec += ts.tv_nsec / 1000000000;
	ts.tv_nsec = ts.tv_nsec % 1000000000;

	call_mutex_lock(mutex, buf, buf_len);
	if ((ret = pthread_cond_timedwait(cond, mutex, &ts)) != ETIMEDOUT) {
#if DEBUG
		tst_resm(TINFO,
			 "Loop %d of 1000000: pthread_cond_timedwait() didn't timeout",
			 i);
		tst_resm(TINFO,
			 "You may want to try reducing the value of NSECS_TO_WAIT (currently=%d)",
			 NSECS_TO_WAIT);
#endif
	}
	call_mutex_unlock(mutex, buf, buf_len);

}

void *run(void *arg)
{
	char buf[1024];

	while (1) {
		call_mutex_lock(&ack, buf, sizeof(buf));
		idle_count = 1;
		call_cond_signal(&parent, buf, sizeof(buf));
		call_mutex_lock(&req, buf, sizeof(buf));
		call_mutex_unlock(&ack, buf, sizeof(buf));

		call_mutex_lock(&wait, buf, sizeof(buf));
		call_cond_signal(&parent, buf, sizeof(buf));
		call_mutex_unlock(&wait, buf, sizeof(buf));

		call_cond_wait(&child, &req, buf, sizeof(buf));
		call_mutex_unlock(&req, buf, sizeof(buf));
	}
}

void create_child_thread(char *buf, size_t buf_len)
{
	pthread_attr_t attr;
	pthread_t child_tid;
	int ret;

	if ((ret = pthread_attr_init(&attr)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_attr_init failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
	if ((ret = pthread_attr_setdetachstate(&attr,
					       PTHREAD_CREATE_DETACHED)) != 0) {
		tst_brkm(TBROK, cleanup,
			 "pthread_attr_setdetachstate failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
	if ((ret = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_attr_setscope failed: %s",
			 strerror_r(ret, buf, buf_len));
	}

	if ((ret = pthread_create(&child_tid, &attr, run, NULL)) != 0) {
		tst_brkm(TBROK, cleanup, "pthread_create failed: %s",
			 strerror_r(ret, buf, buf_len));
	}
}

void trap_alarm(int sig)
{
	tst_brkm(TFAIL, cleanup, "Test hang longer than %d sec detected",
		 MAXTIME);
}

static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s -l loops\n", progname);
	fprintf(stderr, "\t-l specify the number of loops to carry out\n");
}

int main(int argc, char **argv)
{
#ifdef USING_NPTL
	char buf[1024];
	int i;
	extern char *optarg;
	int numloops = NUMLOOPS;

	while ((i = getopt(argc, argv, "l:")) != -1) {
		switch (i) {
		case 'l':
			if (optarg)
				numloops = atoi(optarg);
			else
				fprintf(stderr,
					"%s: option -l requires an argument\n",
					argv[0]);
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}

	signal(SIGALRM, trap_alarm);
	alarm(MAXTIME);

	call_mutex_init(&req, buf, sizeof(buf));
	call_mutex_init(&ack, buf, sizeof(buf));
	call_mutex_init(&wait, buf, sizeof(buf));
	call_cond_init(&parent, buf, sizeof(buf));
	call_cond_init(&child, buf, sizeof(buf));

	call_mutex_lock(&ack, buf, sizeof(buf));

	create_child_thread(buf, sizeof(buf));

	tst_resm(TINFO, "Starting test, please wait.");
	for (i = 0; i < numloops; i++) {
		while (idle_count == 0) {
			call_cond_wait(&parent, &ack, buf, sizeof(buf));
		};
		idle_count = 0;
		call_mutex_unlock(&ack, buf, sizeof(buf));

		do_timedwait(&parent, &wait, buf, sizeof(buf), i);

		call_mutex_lock(&req, buf, sizeof(buf));
		call_cond_signal(&child, buf, sizeof(buf));
		call_mutex_unlock(&req, buf, sizeof(buf));
#ifdef DEBUG
		tst_resm(TINFO, "Success in loop %d", i);
#else
		if (((i % (numloops / 10)) == 0) && (i != 0))
			tst_resm(TINFO, "Success thru loop %d of %i", i,
				 numloops);
#endif
		call_mutex_lock(&ack, buf, sizeof(buf));
	}

	alarm(0);
	tst_resm(TPASS, "Test completed successfully!");
	cleanup();

#else
	tst_brkm(TCONF, NULL,
		 "Skipping Execution - This system is not using NPTL");
#endif

	return 1;
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup()
{

	tst_exit();
}
