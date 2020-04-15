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
/* File:        cpuctl_task_def01.c                                           */
/*                                                                            */
/* Description: This is a c program that runs as a default task in a default  */
/*              group to create an ideal scenario. In this default group all  */
/*              the system tasks will be running along with this spinning task*/
/*              The file is to be used by tests 1-3.                          */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   cpu_controller_tests                                          */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file cpuctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created-     14/11/2008 -Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
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

#include "../libcontrollers/libcontrollers.h"
#include "test.h"		/* LTP harness APIs */
#include "safe_macros.h"

#ifdef DEBUG
#define dbg(x...)	printf(x);
#else
#define dbg(x...)	do {}	while (0)
#endif

#define TIME_INTERVAL	30	/* Time interval in seconds */
#define NUM_INTERVALS	3	/* How many iterations of TIME_INTERVAL */
#define NUM_SETS	4	/* How many share values (with same ratio) */
#define MULTIPLIER   	10	/* Rate at which share value gets multiplied */

char *TCID = "cpu_controller_tests";
int TST_TOTAL = 1;
pid_t scriptpid;
char path[FILENAME_MAX] = "/dev/cpuctl";

extern void cleanup(void)
{
	kill(scriptpid, SIGUSR1);	/* Inform the shell to do cleanup */
	/* Report exit status */
}

int main(int argc, char *argv[])
{

	int num_cpus, test_num, len;	/* Total time = TIME_INTERVAL*num_cpus */
	char mygroup[FILENAME_MAX], mytaskfile[FILENAME_MAX];
	char mysharesfile[FILENAME_MAX], ch;
	pid_t pid;
	int my_group_num;	/* A number attached with a group */
	int fd;			/* To open a fifo for synchronization */
	int first_counter = 0;	/* To take n number of readings */
	int second_counter = 0;	/* no of times shares have changed */
	double total_cpu_time;	/* Accumulated cpu time */
	double delta_cpu_time;	/* Time the task could run on cpu(s) */
	double prev_cpu_time = 0;
	double exp_cpu_time;	/* Exp time in % by shares calculation */
	struct rusage cpu_usage;
	time_t current_time, prev_time, delta_time;
	unsigned long int myshares = 2, baseshares = 1000;
	unsigned int fmyshares, num_tasks;
	struct sigaction newaction, oldaction;

	num_cpus = 0;
	test_num = 0;
	my_group_num = -1;

	/* Signal handling for alarm */
	sigemptyset(&newaction.sa_mask);
	newaction.sa_handler = signal_handler_alarm;
	newaction.sa_flags = 0;
	sigaction(SIGALRM, &newaction, &oldaction);

	/* Check if all parameters passed are correct */
	if ((argc < 5) || ((my_group_num = atoi(argv[1])) <= 0) ||
	    ((scriptpid = atoi(argv[3])) <= 0) ||
	    ((num_cpus = atoi(argv[4])) <= 0) ||
	    (test_num = atoi(argv[5])) <= 0)
		tst_brkm(TBROK, cleanup, "Invalid input parameters\n");

	if (test_num == 1)
		myshares *= my_group_num;
	else if (test_num == 3)
		myshares = baseshares;
	else
		tst_brkm(TBROK, cleanup, "Wrong Test num passed. Exiting.\n");

	sprintf(mygroup, "%s", argv[2]);
	sprintf(mytaskfile, "%s", mygroup);
	sprintf(mysharesfile, "%s", mygroup);
	strcat(mytaskfile, "/tasks");
	strcat(mysharesfile, "/cpu.shares");
	pid = getpid();
	write_to_file(mytaskfile, "a", pid);	/* Assign task to it's group */
	write_to_file(mysharesfile, "w", myshares);
	dbg("Default task's initial shares = %u", myshares);

	fd = SAFE_OPEN(cleanup, "./myfifo", 0);

	read(fd, &ch, 1);	/* To fire all the tasks up at the same time */

	/*
	 * We need not calculate the expected % cpu time of this task, as
	 * neither it is required nor it can be predicted as there are idle
	 * system tasks (and others too) in this group.
	 */
	FLAG = 0;
	total_shares = 0;
	shares_pointer = &total_shares;
	len = strlen(path);
	if (!strncpy(fullpath, path, len))
		tst_brkm(TBROK, cleanup,
			 "Could not copy directory path %s ", path);

	if (scan_shares_files(shares_pointer) != 0)
		tst_brkm(TBROK, cleanup,
			 "From function scan_shares_files in %s ", fullpath);

	/* return val -1 in case of function error, else 2 is min share value */
	if ((fmyshares = read_shares_file(mysharesfile)) < 2)
		tst_brkm(TBROK, cleanup,
			 "in reading shares files  %s ", mysharesfile);

	if ((read_file(mytaskfile, GET_TASKS, &num_tasks)) < 0)
		tst_brkm(TBROK, cleanup,
			 "in reading tasks files  %s ", mytaskfile);

	exp_cpu_time = (double)(fmyshares * 100) / (total_shares * num_tasks);

	prev_time = time(NULL);	/* Note down the time */

	while (1) {
		/*
		 * Need to run some cpu intensive task, which also
		 * frequently checks the timer value
		 */
		double f = 274.345, mytime;	/*just a float number for sqrt */
		alarm(TIME_INTERVAL);
		timer_expired = 0;
		/*
		 * Let the task run on cpu for TIME_INTERVAL. Time of this
		 * operation should not be high otherwise we can exceed the
		 * TIME_INTERVAL to measure cpu usage
		 */
		while (!timer_expired)
			f = sqrt(f * f);

		current_time = time(NULL);
		/* Duration in case its not exact TIME_INTERVAL */
		delta_time = current_time - prev_time;

		getrusage(0, &cpu_usage);
		/* total_cpu_time = total user time + total sys time */
		total_cpu_time = (cpu_usage.ru_utime.tv_sec +
				  cpu_usage.ru_utime.tv_usec * 1e-6 +
				  cpu_usage.ru_stime.tv_sec +
				  cpu_usage.ru_stime.tv_usec * 1e-6);
		delta_cpu_time = total_cpu_time - prev_cpu_time;

		prev_cpu_time = total_cpu_time;
		prev_time = current_time;

		/* calculate % cpu time each task gets */
		if (delta_time > TIME_INTERVAL)
			mytime = (delta_cpu_time * 100) /
			    (delta_time * num_cpus);
		else
			mytime = (delta_cpu_time * 100) /
			    (TIME_INTERVAL * num_cpus);

		/*
		 * Lets print the results. The exp cpu time calculated may not
		 * be correct due to running system tasks at the moment
		 */
		fprintf(stdout, "DEF TASK:CPU TIME{calc:-%6.2f(s)"
			" i.e. %6.2f(%%) exp:-%6.2f(%%)} with %lu(shares)"
			" in %lu (s) INTERVAL\n", delta_cpu_time, mytime,
			exp_cpu_time, myshares, delta_time);
		first_counter++;

		/* Take n sets of readings for each shares value */
		if (first_counter >= NUM_INTERVALS) {
			first_counter = 0;
			second_counter++;
			if (second_counter >= NUM_SETS)
				exit(0);	/* This task is done */

			/* Keep same ratio but change values */
			if (test_num == 1) {
				myshares = MULTIPLIER * myshares;
				write_to_file(mysharesfile, "w", myshares);
			}
			/* No need to change shares for def task for test 3 */

		}		/* end if */
	}			/* end while */
}				/* end main */
