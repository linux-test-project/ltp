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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        cpuctl_test02.c                                               */
/*                                                                            */
/* Description: This is a c program that tests the cpucontroller fairness of  */
/*              scheduling the tasks according to their group shares. This    */
/*              testcase tests the ability of the cpu controller to provide   */
/*              fairness for share values (absolute).                         */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   cpu_controller_test02                                         */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file cpuctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar sudhirkumarmalik@in.ibm.com                      */
/*                                                                            */
/* History:                                                                   */
/* Created-     20/12/2007 -Sudhir Kumar sudhirkumarmalik@in.ibm.com          */
/*                                                                            */
/******************************************************************************/

/* Standard Include Files */
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
#include <time.h>
#include <unistd.h>

#include "test.h"		/* LTP harness APIs*/

#define TIME_INTERVAL	60	/* Time interval in seconds*/
#define NUM_INTERVALS	4       /* How many iterations of TIME_INTERVAL */

extern int Tst_count;
char *TCID = "cpu_controller_test04";
int TST_TOTAL = 1;
pid_t scriptpid;
extern void
cleanup()
{
	kill (scriptpid, SIGUSR1);/* Inform the shell to do cleanup*/
	tst_exit ();		/* Report exit status*/
}

int write_to_file (char * file, const char* mode, unsigned int value);
void signal_handler_alarm (int signal );
int timer_expired = 0;

int main(int argc, char* argv[])
{

	int test_num, num_cpus;			/* To calculate cpu time in %*/
	char mygroup[32], mytaskfile[32], ch;
	const char *var1 ="GROUP_NUM", *var2 = "MYGROUP", *var3 = "SCRIPT_PID", *var4 = "NUM_CPUS", *var5 = "TEST_NUM";
	char *group_num_p, *mygroup_p, *script_pid_p, *num_cpus_p, *test_num_p;
	pid_t pid;
	int my_group_num,	        /* A number attached with a group*/
		fd,          	        /* A descriptor to open a fifo for synchronized start*/
		counter =0; 	 	/* To take n number of readings*/
	double total_cpu_time,  	/* Accumulated cpu time*/
		delta_cpu_time,  	/* Time the task could run on cpu(s) (in an interval)*/
		prev_cpu_time=0;
	struct rusage cpu_usage;
	time_t current_time, prev_time, delta_time;
	struct sigaction newaction, oldaction;
	/* Signal handling for alarm*/
	sigemptyset (&newaction.sa_mask);
	newaction.sa_handler = signal_handler_alarm;
	newaction.sa_flags=0;
	sigaction (SIGALRM, &newaction, &oldaction);

	/* Check if all parameters passed are correct*/
/*	if ((argc < 5) || ((my_group_num = atoi(argv[1])) <= 0) || ((scriptpid = atoi(argv[3])) <= 0) || ((num_cpus = atoi(argv[4])) <= 0))
	{
		tst_brkm (TBROK, cleanup, "Invalid input parameters\n");
	}
*/
	/* Collect the parameters passed by the script*/
	group_num_p = getenv(var1);
	mygroup_p = getenv(var2);
	script_pid_p = getenv(var3);
	num_cpus_p = getenv(var4);
	test_num_p = getenv(var5);
	if ((test_num_p != NULL) && ((test_num = atoi(test_num_p)) == 3))
	{
		if ((group_num_p== NULL)||(mygroup_p == NULL)||(script_pid_p == NULL)||(num_cpus_p == NULL))
		{
			tst_brkm (TBROK, cleanup, "Invalid other input parameters\n");
		}
		else
		{
			my_group_num	 = atoi(group_num_p);
			scriptpid	 = atoi(script_pid_p);
			num_cpus	 = atoi (num_cpus_p);
			sprintf(mygroup,"%s", mygroup_p);
		}
	}
	else
	{
		tst_brkm (TBROK, cleanup, "Invalid test number passed\n");
	}

	sprintf(mytaskfile, "%s", mygroup);
	strcat (mytaskfile,"/tasks");
	pid = getpid();
	write_to_file (mytaskfile, "a", pid);    /* Assign the task to it's group*/

	fd = open ("./myfifo", 0);
	if (fd == -1)
	{
		tst_brkm (TBROK, cleanup, "Could not open fifo for synchronization");
	}

	read (fd, &ch, 1);	         /* To block all tasks here and fire them up at the same time*/
	prev_time = time (NULL);	 /* Note down the time*/

	while (1)
	{
		/* Need to run some cpu intensive task, which also frequently checks the timer value*/
		double f = 274.345, mytime;	/*just a float number to take sqrt*/
		alarm (TIME_INTERVAL);
		timer_expired = 0;
		while (!timer_expired)	/* Let the task run on cpu for TIME_INTERVAL*/
			f = sqrt (f*f); /* Time of this operation should not be high otherwise we can
					 * exceed the TIME_INTERVAL to measure cpu usage
					 */
		current_time = time (NULL);
		delta_time = current_time - prev_time;	/* Duration in case its not exact TIME_INTERVAL*/

		getrusage (0, &cpu_usage);
		total_cpu_time = (cpu_usage.ru_utime.tv_sec + cpu_usage.ru_utime.tv_usec * 1e-6 + /* user time*/
				cpu_usage.ru_stime.tv_sec + cpu_usage.ru_stime.tv_usec * 1e-6) ;  /* system time*/
		delta_cpu_time = total_cpu_time - prev_cpu_time;

		prev_cpu_time = total_cpu_time;
		prev_time = current_time;
		if (delta_time > TIME_INTERVAL)
			mytime =  (delta_cpu_time * 100) / (delta_time * num_cpus);
		else
			mytime =  (delta_cpu_time * 100) / (TIME_INTERVAL * num_cpus);

		fprintf (stdout,"PID: %u\tgroup-%d:cpu time ---> %6.3f\%(%fs)\tinterval:%lu\n",getpid(),my_group_num, mytime, delta_cpu_time, delta_time);
		counter++;

		if (counter >= NUM_INTERVALS)	 /* Take n sets of readings for each shares value*/
			exit (0);		/* This task is done with its job*/

        }/* end while*/
}/* end main*/


int write_to_file (char *file, const char *mode, unsigned int value)
{
	FILE *fp;
	fp = fopen (file, mode);
	if (fp == NULL)
	{
		tst_brkm (TBROK, cleanup, "Could not open file %s for writing", file);
	}
	fprintf (fp, "%u\n", value);
	fclose (fp);
	return 0;
}

void signal_handler_alarm (int signal)
{
	timer_expired = 1;
}
