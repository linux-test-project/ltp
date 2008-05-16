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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *      Use "-j" to enable jvm simulator.
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
#include <libjvmsim.h>

/* #define CLOCK_TO_USE CLOCK_MONOTONIC */
#define CLOCK_TO_USE CLOCK_REALTIME

#define START_MAX	3000
#define REPORT_MIN	1000000

static int run_jvmsim = 0;
static unsigned int max_window = 0; /* infinite, don't use a window */

void usage(void)
{
	rt_help();
 	printf("gtod_infinite specific options:\n");
  	printf("  -j            enable jvmsim\n");
 	printf("  -wWINDOW      iterations in max value window (default inf)\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
		case 'j':
			run_jvmsim = 1;
			break;
		case 'h':
			usage();
			exit(0);
		case 'w':
			max_window = atoi(v);
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

int main(int argc, char *argv[])
{
	int/* i,*/ rc;
	struct timespec ts, p_ts;
	nsec_t s_time, e_time, diff_time;
	nsec_t max_time = START_MAX;
//	cpu_set_t mask;
	struct sched_param param;
	time_t tt;
	unsigned int wi;
	setup();

/*
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc) {
		perror("sched_setaffinity");
		exit(1);
	}
*/
	rt_init("jhw:", parse_args, argc, argv);

	if (run_jvmsim) {
		printf("jvmsim enabled\n");
		jvmsim_init();
	} else {
		printf("jvmsim disabled\n");
	}

	if (max_window > 0) {
		printf("%d iterations in max calculation window\n", 
		       max_window);
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

	wi = 0;
	while(1) {
		rc = clock_gettime(CLOCK_TO_USE, &ts);
		if (rc) {
			perror("clock_gettime");
			exit(1);
		}

		ts_to_nsec(&p_ts, &s_time);
		ts_to_nsec(&ts, &e_time);

		diff_time = e_time - s_time;

		if (max_window > 0 ||
		          ((diff_time > max_time) || 
			   (diff_time > REPORT_MIN))) {
			if (diff_time > max_time)
				max_time = diff_time;
			
			if (max_window == 0 || ++wi == max_window) {
				tt = (time_t)ts.tv_sec;
				printf("Task delayed for %lld nsec!!! %s",
				       max_time, ctime(&tt));
				fflush(stdout);

				if (wi == max_window) {
					max_time = 0;
					wi = 0;
				}
			}

			rc = clock_gettime(CLOCK_TO_USE, &p_ts);
			if (rc) {
				perror("clock_gettime");
				exit(1);
			}
		} else {
			p_ts = ts;
		}
	}
	return 0;
}
