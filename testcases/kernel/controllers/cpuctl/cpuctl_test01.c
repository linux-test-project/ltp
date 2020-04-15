/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
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
/* File:        cpuctl_test01.c                                               */
/*                                                                            */
/* Description: This is a c program that tests the cpucontroller fairness of  */
/*              scheduling the tasks according to their group shares. This    */
/*              testcase tests the ability of the cpu controller to provide   */
/*              fairness for share values (absolute).                         */
/*                                                                            */
/* Total Tests: 3                                                             */
/*                                                                            */
/* Test 01:     Tests if fairness persists among different runs               */
/* Test 02:     Tests fairness with respect to absolute share values          */
/* Test 03:     Granularity test with respect to shares values                */
/*                                                                            */
/* Test Name:   cpu_controller_test01                                         */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file cpuctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created-     20/12/2007 -Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "../libcontrollers/libcontrollers.h"
#include "test.h"		/* LTP harness APIs */
#include "safe_macros.h"

#define TIME_INTERVAL	30	/* Time interval in seconds */
#define NUM_INTERVALS	3	/* How many iterations of TIME_INTERVAL */
#define NUM_SETS	4	/* How many share values (with same ratio) */
#define MULTIPLIER   	10	/* decides the rate at which share value gets multiplied */
#define GRANULARITY    5	/* % value by which shares of a group changes */
char *TCID = "cpuctl_test01";
int TST_TOTAL = 1;
pid_t scriptpid;
char path[] = "/dev/cpuctl";
extern void cleanup(void)
{
	kill(scriptpid, SIGUSR1);	/* Inform the shell to do cleanup */
	tst_exit();		/* Report exit status */
}

int main(int argc, char *argv[])
{

	int num_cpus;
	int test_num;
	int len;		/* Total time = TIME_INTERVAL *num_cpus in the machine */
	char mygroup[FILENAME_MAX], mytaskfile[FILENAME_MAX];
	char mysharesfile[FILENAME_MAX], ch;
	pid_t pid;
	gid_t my_group_num;	/* A number attached with a group */
	int fd;			/* A descriptor to open a fifo for synchronized start */
	int first_counter = 0;	/* To take n number of readings */
	int second_counter = 0;	/* To track number of times the base value of shares has been changed */
	double total_cpu_time,	/* Accumulated cpu time */
	 delta_cpu_time,	/* Time the task could run on cpu(s) (in an interval) */
	 prev_cpu_time = 0;
	double exp_cpu_time;	/* Expected time in % as obtained by shares calculation */
	struct rusage cpu_usage;
	time_t current_time, prev_time, delta_time;
	unsigned long int myshares = 2, baseshares = 1000;	/* Simply the base value to start with */
	unsigned int fmyshares, num_tasks;	/* f-> from file. num_tasks is tasks in this group */
	struct sigaction newaction, oldaction;

	my_group_num = -1;
	num_cpus = 0;
	test_num = 0;

	/* Signal handling for alarm */
	sigemptyset(&newaction.sa_mask);
	newaction.sa_handler = signal_handler_alarm;
	newaction.sa_flags = 0;
	sigaction(SIGALRM, &newaction, &oldaction);

	/* Check if all parameters passed are correct */
	if ((argc < 5) || ((my_group_num = atoi(argv[1])) <= 0)
	    || ((scriptpid = atoi(argv[3])) <= 0)
	    || ((num_cpus = atoi(argv[4])) <= 0)
	    || (test_num = atoi(argv[5])) <= 0) {
		tst_brkm(TBROK, cleanup, "Invalid input parameters\n");
	}

	if (test_num == 1)	/* Test 01 & Test 02 */
		myshares *= my_group_num;
	else if (test_num == 3)	/* Test 03 */
		myshares = baseshares;
	else {
		tst_brkm(TBROK, cleanup,
			 "Wrong Test number passed. Exiting Test...\n");
	}

	sprintf(mygroup, "%s", argv[2]);
	sprintf(mytaskfile, "%s", mygroup);
	sprintf(mysharesfile, "%s", mygroup);
	strcat(mytaskfile, "/tasks");
	strcat(mysharesfile, "/cpu.shares");
	pid = getpid();
	write_to_file(mytaskfile, "a", pid);	/* Assign the task to it's group */
	write_to_file(mysharesfile, "w", myshares);

	fd = SAFE_OPEN(cleanup, "./myfifo", 0);

	fprintf(stdout, "\ntask-%d SHARES=%lu\n", my_group_num, myshares);
	read(fd, &ch, 1);	/* To block all tasks here and fire them up at the same time */

	/*
	 * We now calculate the expected % cpu time of this task by getting
	 * it's group's shares, the total shares of all the groups and the
	 * number of tasks in this group.
	 */
	FLAG = 0;
	total_shares = 0;
	shares_pointer = &total_shares;
	len = strlen(path);
	if (!strncpy(fullpath, path, len))
		tst_brkm(TBROK, cleanup, "Could not copy directory path %s ",
			 path);

	if (scan_shares_files(shares_pointer) != 0)
		tst_brkm(TBROK, cleanup,
			 "From function scan_shares_files in %s ", fullpath);

	/* return val: -1 in case of function error, else 2 is min share value */
	if ((fmyshares = read_shares_file(mysharesfile)) < 2)
		tst_brkm(TBROK, cleanup, "in reading shares files  %s ",
			 mysharesfile);

	if ((read_file(mytaskfile, GET_TASKS, &num_tasks)) < 0)
		tst_brkm(TBROK, cleanup, "in reading tasks files  %s ",
			 mytaskfile);

	exp_cpu_time = (double)(fmyshares * 100) / (total_shares * num_tasks);

	prev_time = time(NULL);	/* Note down the time */

	while (1) {
		/* Need to run some cpu intensive task, which also frequently checks the timer value */
		double f = 274.345, mytime;	/*just a float number to take sqrt */
		alarm(TIME_INTERVAL);
		timer_expired = 0;
		while (!timer_expired)	/* Let the task run on cpu for TIME_INTERVAL */
			f = sqrt(f * f);	/* Time of this operation should not be high otherwise we can
						 * exceed the TIME_INTERVAL to measure cpu usage
						 */
		current_time = time(NULL);
		delta_time = current_time - prev_time;	/* Duration in case its not exact TIME_INTERVAL */

		getrusage(0, &cpu_usage);
		total_cpu_time = (cpu_usage.ru_utime.tv_sec + cpu_usage.ru_utime.tv_usec * 1e-6 +	/*user */
				  cpu_usage.ru_stime.tv_sec + cpu_usage.ru_stime.tv_usec * 1e-6);	/*sys */
		delta_cpu_time = total_cpu_time - prev_cpu_time;

		prev_cpu_time = total_cpu_time;
		prev_time = current_time;

		/* calculate % cpu time each task gets */
		if (delta_time > TIME_INTERVAL)
			mytime =
			    (delta_cpu_time * 100) / (delta_time * num_cpus);
		else
			mytime =
			    (delta_cpu_time * 100) / (TIME_INTERVAL * num_cpus);

		fprintf(stdout, "task-%d:CPU TIME{calc:-%6.2f(s)i.e. %6.2f(%%) exp:-%6.2f(%%)}\
with %lu(shares) in %lu (s) INTERVAL\n", my_group_num, delta_cpu_time, mytime,
			exp_cpu_time, myshares, delta_time);
		first_counter++;

		if (first_counter >= NUM_INTERVALS) {	/* Take n sets of readings for each shares value */
			first_counter = 0;
			second_counter++;
			if (second_counter >= NUM_SETS)
				exit(0);	/* This task is done with its job */

			/* Change share values depending on the test_num */
			if (test_num == 1) {
				/* Keep same ratio but change values */
				myshares = MULTIPLIER * myshares;
			} else {
				/* Increase for odd task and decrease for even task */
				if (my_group_num % 2)
					myshares +=
					    baseshares * GRANULARITY / 100;
				else
					myshares -=
					    baseshares * GRANULARITY / 100;
			}
			write_to_file(mysharesfile, "w", myshares);
			if (test_num == 3) {
				/*
				 * Read the shares file and again calculate the cpu fraction
				 * No need to read tasks file as we do not migrate tasks
				 * No need to scan all shares file as total shares are const
				 */
				if ((fmyshares =
				     read_shares_file(mysharesfile)) < 2)
					tst_brkm(TBROK, cleanup,
						 "in reading shares files  %s ",
						 mysharesfile);
				exp_cpu_time =
				    (double)(fmyshares * 100) / (total_shares *
								 num_tasks);
			}

			fprintf(stdout, "\ntask-%d SHARES=%lu\n", my_group_num,
				myshares);
		}		/* end if */
	}			/* end while */
}				/* end main */
