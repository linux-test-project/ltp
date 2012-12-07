/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006, 2008
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
 *      gtod_infinite.c
 *
 * DESCRIPTION
 *       This 'test' is designed to run forever.  It must manually be killed,
 *       so it is not ideally suited tobe part of a validation suite of tests.
 *       This test was initially designed to look for 'delays' between two
 *       calls to clock_gettime(), and helped locate SMI induced delays on
 *       several hardware platforms.
 *
 *       As mentioned above, this test is designed to be run on a system for
 *       an unspecified period of time.  It would not be unusual to let this
 *       test run for several days.
 *
 *       During startup, the test will print out the 'maximum' delay between
 *       clock_gettime() calls.  It starts with a predefined maximum of
 *       START_MAX(300ns) to eliminate noise during startup.  In addition,
 *       it will print out every delay that exceeds REPORT_MIN (1000000ns).
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Aug-17: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <librttest.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

#define CLOCK_TO_USE CLOCK_MONOTONIC

#define START_MAX	3000
#define REPORT_MIN	1000000

static unsigned int max_window = 0;	/* infinite, don't use a window */
static unsigned int test_duration = 0;	/* infinite duration */
static int test_stop = 0;	/* 1 to stop */

void usage(void)
{
	rt_help();
	printf("gtod_infinite specific options:\n");
	printf
	    ("  -wWINDOW      iterations in max value window (default inf)\n");
	printf("  -tDURATION    test duration in finite hours (default inf)\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'w':
		max_window = atoi(v);
		break;
	case 't':
		test_duration = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

void alarm_handler(int sig)
{
	/* Stop test execution */
	test_stop = 1;
}

int main(int argc, char *argv[])
{
	int /* i, */ rc;
	struct timespec ts, p_ts;
	nsec_t s_time, e_time, diff_time;
	nsec_t max_time = START_MAX;
//      cpu_set_t mask;
	struct sched_param param;
	time_t tt;
	unsigned int wi;
	struct sigaction sact;
	setup();

	/* Set signal handler for SIGALRM */
	sigfillset(&sact.sa_mask);
	sact.sa_handler = alarm_handler;
	rc = sigaction(SIGALRM, &sact, NULL);
	if (rc) {
		perror("sigaction");
		exit(1);
	}
/*
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc) {
		perror("sched_setaffinity");
		exit(1);
	}
*/
	rt_init("hw:t:", parse_args, argc, argv);

	mlockall(MCL_CURRENT | MCL_FUTURE);

	if (max_window > 0) {
		printf("%d iterations in max calculation window\n", max_window);
	}

	param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 80;
	rc = sched_setscheduler(0, SCHED_FIFO, &param);
	if (rc) {
		perror("sched_setscheduler");
		exit(1);
	}

	rc = clock_gettime(CLOCK_TO_USE, &p_ts);
	if (rc) {
		perror("clock_gettime");
		exit(1);
	}

	/* Set alarm for test duration, if specified */
	if (test_duration > 0) {
		rc = alarm(test_duration * 60 * 60);
		if (rc) {
			perror("alarm");
			exit(1);
		}
	}

	wi = 0;
	while (test_stop != 1) {
		rc = clock_gettime(CLOCK_TO_USE, &p_ts);
		rc = clock_gettime(CLOCK_TO_USE, &ts);
		if (rc) {
			perror("clock_gettime");
			exit(1);
		}

		ts_to_nsec(&p_ts, &s_time);
		ts_to_nsec(&ts, &e_time);

		diff_time = e_time - s_time;

		if (max_window > 0 ||
		    ((diff_time > max_time) || (diff_time > REPORT_MIN))) {
			if (diff_time > max_time)
				max_time = diff_time;

			if (max_window == 0 || ++wi == max_window) {
				tt = (time_t) ts.tv_sec;
				printf("Task delayed for %lld nsec!!! %s",
				       max_time, ctime(&tt));
				fflush(stdout);

				if (wi == max_window) {
					max_time = 0;
					wi = 0;
				}
			}

		}
	}

	return 0;
}
