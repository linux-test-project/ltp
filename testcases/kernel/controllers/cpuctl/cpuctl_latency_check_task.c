/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        cpuctl_latency_check_task.c                                   */
/*                                                                            */
/* Description: This is a c program that runs a task which does frequent sleep*/
/*              on a busy machine and checks if there is any added latency    */
/*              The file is to be used by script                              */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   cpu_controller_latency_tests                                  */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file cpuctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created-     26/11/2008 -Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "../libcontrollers/libcontrollers.h"

/* #define VERBOSE	1 to print verbose output */

#ifdef VERBOSE
#define verbose(x...) printf(x)
#else
#define verbose(x...) do {} while (0);
#endif

#define NUM_TIMES 200		/* How many intervals you want to check */
#define INTERVALS 1		/* How many milliseconds interval in iterations */
#define USECONDS  1000		/* microseconds to sleep */
#define info	printf("The results FAIL is just intuitive and not exact" \
		" failure. Please look at cpuctl_testplan.txt in the test directory.\n");

char *TCID = "cpuctl_latency_tests";
int TST_COUNT = 1;
int TST_TOTAL = 1;
pid_t script_pid;

int main(int argc, char *argv[])
{
	int count, i = 0, iteration = 0;
	int fail = 0, ALLOWED;
	char mytaskfile[FILENAME_MAX];
	int test_num;
	struct timeval prev_time, cur_time;
	unsigned int actual, actual_s, actual_us, sleeptime;
	unsigned int delta, delta_max = 0;
	pid_t script_pid;

	if ((argc < 4) || (argc > 5)) {
		printf("Invalid #args received from script. Exiting test..\n");
		exit(1);
	}

	test_num = atoi(argv[1]);
	script_pid = (pid_t) atoi(argv[2]);
	ALLOWED = atoi(argv[3]);
	if ((test_num < 0) || (script_pid < 0) || (ALLOWED < 0)) {
		printf("Invalid args received from script. Exiting test..\n");
		exit(1);
	}

	if (test_num == 2) {
		strncpy(mytaskfile, argv[4], FILENAME_MAX);
		strncat(mytaskfile, "/tasks",
			FILENAME_MAX - strlen(mytaskfile) - 1);
		write_to_file(mytaskfile, "a", getpid());

		/* Give a chance to other tasks too to go to their class */
		sleep(8);
	}

	printf("TINFO \tThe latency check task started\n");

	/* Let us start capturing the time now */
	for (count = NUM_TIMES; count >= 0; count -= INTERVALS) {
		if (gettimeofday(&prev_time, NULL) == -1)
			perror("In Iteration no 1 \n");
		/* sleep for specified time */
		sleeptime = count * USECONDS;
		usleep(sleeptime);

		if (gettimeofday(&cur_time, NULL) == -1)
			perror("In Iteration no 1 \n");

		/* Get the actual difference */
		actual_s = cur_time.tv_sec - prev_time.tv_sec;
		actual_us = cur_time.tv_usec - prev_time.tv_usec;
		actual = 1e6 * actual_s + actual_us;
		delta = actual - sleeptime;

		/*  capture the maximum latency observed */
		if (delta >= delta_max) {
			delta_max = delta;
			iteration = i;
		}

		if (delta > ALLOWED)
			fail = 1;

		verbose("Iteration %d: Exp(us) =%u, Actual =%u delta = %u\n",
			i++, sleeptime, actual, delta);
	}

	if (fail) {
		printf("FAIL \tThe Latency test %d failed\n", test_num);
		printf("Max latency observed = %u in Iteration %d\n",
		       delta_max, iteration);
		info;
	} else {
		printf("PASS \tThe Latency test %d passed\n", test_num);
		printf("Max latency observed = %u microsec in Iteration %d\n",
		       delta_max, iteration);
	}
	return fail;
}
