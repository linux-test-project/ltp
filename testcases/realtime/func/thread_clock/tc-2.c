/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *    tc-2.c
 *
 * DESCRIPTION
 *    Check if clock_gettime is working properly.
 *    This test creates NUMSLEEP threads that just sleep and NUMWORK threads
 *    that spend time on the CPU. It then reads the thread cpu clocks of all
 *    these threads and compares the sum of thread cpu clocks with the process
 *    cpu clock value. The test expects that:
 *    the cpu clock of every sleeping thread shows close to zero value.
 *    sum of cpu clocks of all threads is comparable with the process cpu clock.
 *
 *
 * USAGE:
 *    Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *    Sripathi Kodi <sripathik@in.ibm.com>
 *
 * HISTORY
 *    2007-Apr-04:  Initial version by Sripathi Kodi <sripathik@in.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <librttest.h>

#define NS_PER_SEC 1000000000
#define THRESHOLD 0.5		/* 500 milliseconds */
#define NUMSLEEP 5
#define NUMWORK 2

struct timespec sleepts[NUMSLEEP];
struct timespec workts[NUMWORK];

void usage(void)
{
	rt_help();
	printf("thread_clock specific options:\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

/* Just spend some time on the CPU */
void work(void)
{
	unsigned int i = 0;
	for (i = 0; i < 2147483600; i++) {
		if ((i == i + 1) || (i == i - 1))
			printf("Hey!\n");
	}
}

void *workerthread(void *arg)
{
	struct thread *pthr = (struct thread *)arg;
	int tid = (int)(long)pthr->arg;
	struct timespec *ts = &workts[tid];

#ifdef DEBUG
	printf("Worker thread %d working\n", tid);
#endif
	work();

	if ((clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts)) < 0) {
		perror("clock_gettime: CLOCK_THREAD_CPUTIME_ID: ");
		exit(1);
	}
#ifdef DEBUG
	printf("workerthread %d: AFTER WORK: tv_sec = %ld, tv_nsec = %ld\n",
	       tid, ts->tv_sec, ts->tv_nsec);
#endif
	return NULL;
}

void *sleeperthread(void *arg)
{
	struct thread *pthr = (struct thread *)arg;
	int tid = (int)(long)pthr->arg;
	struct timespec *ts = &sleepts[tid];

#ifdef DEBUG
	printf("Sleeper thread %d sleeping\n", tid);
#endif

	sleep(5);

	if ((clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts)) < 0) {
		perror("clock_gettime: CLOCK_THREAD_CPUTIME_ID: ");
		exit(1);
	}
#ifdef DEBUG
	printf("sleeperthread %d: AFTER SLEEP: tv_sec = %ld, tv_nsec = %ld\n",
	       tid, ts->tv_sec, ts->tv_nsec);
#endif
	return NULL;
}

int checkresult(float proctime)
{
	int i, retval = 0;
	float diff, threadstime = 0;
	for (i = 0; i < NUMSLEEP; i++) {
		/* Sleeping thread should not accumulate more than 1 second of CPU time */
		if (sleepts[i].tv_sec > 0) {
			printf
			    ("Sleeper thread %d time is %f, should have been close to zero. FAIL\n",
			     i,
			     sleepts[i].tv_sec +
			     ((float)sleepts[i].tv_nsec / NS_PER_SEC));
			retval = 1;
		}
		threadstime +=
		    sleepts[i].tv_sec +
		    ((float)sleepts[i].tv_nsec / NS_PER_SEC);
	}
	if (retval)
		return retval;

	for (i = 0; i < NUMWORK; i++) {
		threadstime +=
		    workts[i].tv_sec + ((float)workts[i].tv_nsec / NS_PER_SEC);
	}
	diff = proctime - threadstime;
	if (diff < 0)
		diff = -diff;
	printf("Process: %.4f s\n", proctime);
	printf("Threads: %.4f s\n", threadstime);
	printf("Delta:   %.4f s\n", diff);
	/* Difference between the sum of thread times and process time
	 * should not be more than pass_criteria */
	printf("\nCriteria: Delta < %.4f s\n", pass_criteria);
	printf("Result: ");
	if (diff > pass_criteria) {
		printf("FAIL\n");
		retval = 1;
	} else {
		printf("PASS\n");
	}
	return retval;
}

int main(int argc, char *argv[])
{
	int i, retval = 0;
	struct timespec myts;
	setup();

	pass_criteria = THRESHOLD;
	rt_init("ht:", parse_args, argc, argv);

	/* Start sleeper threads */
	for (i = 0; i < NUMSLEEP; i++) {
		if ((create_other_thread(sleeperthread, (void *)(intptr_t) i)) <
		    0) {
			exit(1);
		}
	}
	printf("\n%d sleeper threads created\n", NUMSLEEP);

	/* Start worker threads */
	for (i = 0; i < NUMWORK; i++) {
		if ((create_other_thread(workerthread, (void *)(intptr_t) i)) <
		    0) {
			exit(1);
		}
	}
	printf("\n%d worker threads created\n", NUMWORK);

	printf("\nPlease wait...\n\n");

	join_threads();
	/* Get the process cpu clock value */
	if ((clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &myts)) < 0) {
		perror("clock_gettime: CLOCK_PROCESS_CPUTIME_ID: ");
		exit(1);
	}
	retval = checkresult(myts.tv_sec + ((float)myts.tv_nsec / NS_PER_SEC));
	return retval;
}
