/*
 *   Copyright (c) International Business Machines  Corp., 2001
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
 */
/*---------------------------------------------------------------------+
|                             sched_driver                             |
| ==================================================================== |
|                                                                      |
| Description:  This program uses system calls to change the           |
|               priorities of the throughput measurement testcases.    |
|               When real-time is in effect, priorities 50 through 64  |
|               are used.  (MAX_PRI and MIN_PRI)  When user-time       |
|               (normal) is in effect, 0-14 (corresponding to nice()   |
|               calls) is used.  The driver only keeps track of        |
|               values from 50 to 64, and the testcases will scale     |
|               them down to 0 to 14 when needed, to change the        |
|               priority of a user-time process.                       |
|                                                                      |
| Algorithm:    o  Parse command line arguments                        |
|               o  Set current priority                                |
|               o  Calcucations (process slots, short/long term slots) |
|               o  Perform throughput tests with high priority         |
|               o  Start long-term testcases                           |
|               o  While time remains                                  |
|                  - Start short-term tests                            |
|                  - Perform throughput tests with new priority        |
|                  - Kill short-term tests                             |
|                  - Increase priority                                 |
|                                                                      |
| Usage:        sched_driver [-s n] [-p n] [-t n] [-d] [-v]            |
|                                                                      |
|               where:                                                 |
|                 -s n  stress percentage                              |
|                 -p n  process slots                                  |
|                 -t n  execution time in hours                        |
|                 -d    enable debugging messages                      |
|                 -v    Turn on verbose mode                           |
|                                                                      |
| Last update:   Ver. 1.15, 4/10/94 23:04:23                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     072889  GEB   Initial draft                               |
|    1.2     120793  JAT   Changes for AIX 4.1                         |
|    1.3     041094  DJK   Rewrote protions...                         |
|    1.4     010402  Manoj Iyer Ported to Linux                        |
|                                                                      |
+---------------------------------------------------------------------*/

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include <limits.h>
#include "sched.h"

/*
 * Defines:
 *
 * MAXPROCS: maximum number of processes
 *
 * PRIINC: priority step value
 *
 * MAX_PRI: highest priority to use
 *
 * MIN_PRI: lowest priority to use
 *
 * DEFAULT_STRESS_PERCENTAGE: stress percentage (process slot multiplier)
 *
 * DEFAULT_PROCESS_SLOTS: number of processes test driver will try and create
 *
 * DEFAULT_TIME: time (hours) for which this test driver will run
 *
 * USAGE: usage statement
 */
#define MAXPROCS   100
#define PRIINC     2
#define MAX_PRI    55		/* was 50 */
#define MIN_PRI    75		/* was 64 */
#define DEFAULT_STRESS_PERCENTAGE	0.5
#define DEFAULT_PROCESS_SLOTS		16
#define DEFAULT_TIME			1.00
#define USAGE "Usage:  %s  [-s n] [-p n] [-t n] [-d] [-v]              \n" \
              "        -s n   stress percentage [0.0<n<1.0] (default 0.5) \n" \
              "        -p n   process slots (default 16)                  \n" \
              "        -t n   execution time in hours (default 1.0 hrs)   \n" \
              "        -d     enable debugging messages                   \n" \
              "        -v     Turn on verbose mode 	                  \n"

/*
 * Global variables:
 *
 * stress_percent: stress percentage
 *
 * :
 *
 * execution_time: execution time in hours
 *
 * debug: (option flag) enables debugging messages
 */
int numprocs,			/* number of process id's in table             */
 procs[MAXPROCS],		/* array of process id's for killing           */
 long_running,			/* number of long term testcases running       */
 short_running;			/* number of short term testcases running      */
float e4user,			/* previous elapsed seconds for tc 4-user      */
 e4real,			/* previous elapsed seconds for tc 4-real      */
 e5user,			/* previous elapsed seconds for tc 5-user      */
 e5real,			/* previous elapsed seconds for tc 5-real      */
 e6user0,			/* previous elapsed seconds for tc 6-user,nf   */
 e6real0,			/* previous elapsed seconds for tc 6-real,nf   */
 e6user1,			/* previous elapsed seconds for tc 6-user,f    */
 e6child;			/* previous elapsed seconds for tc 6-child     */
double stress_percent = DEFAULT_STRESS_PERCENTAGE;
double execution_time = DEFAULT_TIME;
int process_slots = DEFAULT_PROCESS_SLOTS;
int debug = 0;

/*
 * Function prototypes
 */
void startup(time_t);
int start_testcase(char *, char *, char *, char *, char *, char *);
int process_slots_in_use();
int available_user_process_slots();
float measure_test(char *, char *, char *, char *, float *);
void display_line(char *, int, int, float, float *, int);
void perform_throughput_tests(int);
void start_long_term_testcases(int, char *);
void kill_short_term_testcases();
void start_short_term_testcases(int, double, int);
void finishup(long);
void parse_args(int, char **);

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program                                              |
|                                                                      |
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	long runseconds,	/* number of seconds to run */
	 start_time;		/* time at start of driver */
	int current_priority,	/* current priority level for nice */
	 workslots,		/* number of free workslots */
	 long_term_slot_total,	/* workslots for long-term processes */
	 short_term_slot_total;	/* workslots for short-term */

	/*
	 * Parse command line arguments & printer program header...
	 */
	parse_args(argc, argv);
	printf("Scheduler Testsuite Program\n\n");
	fflush(stdout);

	/*
	 * Calculate number of seconds to run, then print out start info
	 */
	runseconds = (long)(execution_time * 60.0 * 60.0);
	start_time = time(NULL);

	startup(start_time);

	/*
	 * Calculate available workslots, long-term, and short-term slots
	 */
	workslots = available_user_process_slots() * stress_percent;
	long_term_slot_total = workslots / 2;
	if (debug) {
		printf("available slots:      %d\n",
		       available_user_process_slots());
		printf("workslots available:  %d\n", workslots);
		printf("stress_percent:       %f\n", stress_percent);
		printf("run-hours:            %f (hrs)\n", execution_time);
		printf("runseconds:           %ld (sec)\n", runseconds);
	}

	/*
	 * Run the first set of tests with an average priority
	 */
	perform_throughput_tests((MAX_PRI + MIN_PRI) / 2);
	fflush(stdout);

	/*
	 * Start the long-term testcases running
	 */
	start_long_term_testcases(long_term_slot_total, argv[2]);
	short_term_slot_total = workslots / 2;
	fflush(stdout);

	/*
	 * Loop while there is still time
	 */
	current_priority = MAX_PRI;
	while ((time(0) - start_time) < runseconds) {

		if (debug)
			printf("current priority: %d\n", current_priority);
		if (debug)
			printf("starting short term tests\n");

		start_short_term_testcases(short_term_slot_total,
					   stress_percent, current_priority);
		fflush(stdout);

		perform_throughput_tests(current_priority);
		fflush(stdout);

		if (debug)
			printf("killing short term tests\n");

		kill_short_term_testcases();
		fflush(stdout);

		if (current_priority + PRIINC > MIN_PRI)
			current_priority = MAX_PRI;
		else
			current_priority += PRIINC;
	}

	/*
	 * Exit with success...
	 */
	finishup(start_time);
	printf("\nsuccessful!\n");
	fflush(stdout);
	return (0);
}

/*------------------------------ startup() ------------------------------*/
/* This procedure opens the , and then outputs some starting	 *
 * information to the screen and .  It also initializes the	 *
 * process id list and other global variables.	 			 *
 *-----------------------------------------------------------------------*/
void startup(time_t start_time)
{
	char tempbuffer[50];	/* temporary buffer to hold names */

	/*
	 * Now output some diagnostic information
	 */
	printf("start time    = %s\n", ctime(&start_time));

	gethostname(tempbuffer, 40);
	printf("host name     = %s\n", tempbuffer);

	printf("user name     = %s\n", getpwuid(geteuid())->pw_name);

	printf("test duration = %4.2f (hours)\n", execution_time);

	printf("test stress   = %4.2f%%%%\n\n", 100 * stress_percent);

	/*
	 * Initialize the global variables
	 */
	numprocs = 0;
	long_running = 0;
	short_running = 0;
	e4user = 0.0;
	e4real = 0.0;
	e5user = 0.0;
	e5real = 0.0;
	e6user0 = 0.0;
	e6real0 = 0.0;
	e6user1 = 0.0;
	e6child = 0.0;
}

/*--------------------------- start_testcase() --------------------------*/
/* This procedure will run a specified testcase by forking a process, and*
 * then running the testcase with it.  It will also store the process id *
 * number in the process id table.  The process id of the child process  *
 * is returned to the calling program.					 *
 *	name1     pathname of testcase to run			         *
 *	name2     filename of testcase to run			         *
 *	param1    parameters to pass to the testcase			 *
 *	param2 			         			         *
 *	param3 			         			         *
 *	param4    if sched_tc6:  fork flag:  0=false, 1=true             *
 *-----------------------------------------------------------------------*/

int start_testcase(char *name1, char *name2, char *param1, char *param2,
		   char *param3, char *param4)
{
	int pid,		/* pid of currently running process */
	 pid_save;		/* saved pid of process */

	/*
	 * Fork a process that will run testcase and save the pid
	 */
	if (debug)
		printf("test: %s %s p1[%s] p2[%s] p3[%s] p4[%s]\n",
		       name1, name2, param1, param2, param3, param4);

	pid_save = pid = fork();

	/*
	 * If the pid returned is -1, fork failed.  If the pid returned is
	 * 0, then the process running is the child process, and we need
	 * to do an 'execl' to run the testcase.  If the pid returned is
	 * anything else, then the parent is running, and we return.
	 */
	switch (pid) {
	case -1:
		exit(-1);
	case 0:
		execl(name1, name2, param1, param2, param3, param4, NULL);
		printf("ERROR: start_testcase():  execl failed.\n");
		exit(-1);
	default:
		break;
	}
	if (debug)
		printf("testcase %s started -- pid is %d\n", name2, pid_save);

	/*
	 * If the process just forked is for a short-term testcase, then
	 * add the process id to the table.
	 */
	if (debug)
		printf("new process: %s ", name2);
	if (strstr(name2, "tc1") || strstr(name2, "tc3")) {
		procs[numprocs] = pid_save;
		numprocs++;
		short_running++;
		if (debug)
			printf("(%d short term)", short_running);
	}
	if (strstr(name2, "tc0") || strstr(name2, "tc2")) {
		long_running++;
		if (debug)
			printf("(%d long term)", long_running);
	}
	if (debug)
		printf("\n");

	return (pid_save);
}

/*------------------------- process_slots_in_use() ----------------------*/
/* This function will return the number of process slots currently in use*
 * by executing the 'ps' command.					 *
 *-----------------------------------------------------------------------*/
int process_slots_in_use()
{
	FILE *psfile;		/* temporary file to hold output of 'ps' command */
	int usedslots;		/* holds the number of used process slots */

	/*
	 * Call the 'ps' command and write the number of process slots to a file
	 */
	if (system("ps -e | wc -l > ps.out") < 0)
		sys_error("system failed", __FILE__, __LINE__);

	/*
	 * Open the output file
	 */
	if ((psfile = fopen("ps.out", "r")) == NULL) {
		exit(-1);
	}

	/*
	 * Read the number of process slots in use from the file
	 */
	fscanf(psfile, "%d", &usedslots);

	/*
	 * Close the output file
	 */
	if (fclose(psfile) == -1) {
		exit(-1);
	}

	/*
	 * Remove the output file
	 */
	if (system("/bin/rm ps.out") < 0)
		sys_error("system failed", __FILE__, __LINE__);

	return (usedslots - 1);
}

/*----------------------- available_user_process_slots() ----------------*/
/* This function returns the total number of available user process slots*
 * by subtracting the process slots currently in use from the maximum	 *
 * possible process slots.						 *
 *-----------------------------------------------------------------------*/
int available_user_process_slots()
{
	int num = process_slots_in_use();

	return ((process_slots < num) ? process_slots : process_slots - num);
}

/*---------------------------- measure_test() ---------------------------*/
/* This function executes a throughput measurement process and waits for *
 * that process to finish.  When finished, it reads the result from a 	 *
 * file and returns that result to the caller.  The file is then deleted.*
 * If sched_tc6 is called, then the second time is also read from the	 *
 * results file and returned to the caller.				 *
 *-----------------------------------------------------------------------*/
float measure_test(name, param1, param2, param3, t2)
char *name,			/* filename of testcase to run */
*param1,			/* user flag:  0=user, 1=real time */
*param2,			/* priority to run the throughput test at */
*param3;			/* if sched_tc6:  fork flag, 0=false, 1=true */
float *t2;			/* if sched_tc6:  second time returned from testcase */
{
	char temp[PATH_MAX],	/* holds pathname and returned floating number */
	 t2asc[50];		/* holds second returned floating number */
	int saved_pid;		/* process id of forked process */
	FILE *datafile;		/* file pointer for temporary file */

	/*
	 * Create the path name to be passed to the start_testcase() function
	 */
	sprintf(temp, "./%s", name);

	/*
	 * Send all the parameters, and start the testcase
	 */
	saved_pid = start_testcase(temp, name, param1,
				   "-lsch.measure", param2, param3);

	/*
	 * Wait for the testcase to finish
	 */
	if (debug)
		printf("waiting on child %d\n", saved_pid);
	while (wait(NULL) != saved_pid) ;

	/*
	 * Open the temporary file to get the returned number of seconds
	 */

	if ((datafile = fopen("sch.measure", "r")) == NULL) {
		sys_error("cannot open sch.measure", __FILE__, __LINE__);
	}

	/*
	 * Read the number of seconds
	 */
	fgets(temp, 50, datafile);
	/*added by mpt
	   printf("sched+driver: measure_test: number of seconds=%s\n",temp)
	   *********** */

	/*
	 * If this is sched_tc6, then there is another number we must return
	 */

	if (strcmp(name, "sched_tc6") == 0) {
		fgets(t2asc, 50, datafile);
		*t2 = atof(t2asc);
	}

	/*
	 * Close the temporary file
	 */
	if (fclose(datafile) != 0) {
		exit(-1);
	}

	/*
	 * Now try to remove the temporary file
	 */
	/*added by MPT
	   printf("measure_test:  REMOVING sch.measure\n");
	   fflush(stdout);
	   if  (system ("rm sch.measure") < 0)
	   sys_error ("system failed", __FILE__, __LINE__);
	 */
	return (atof(temp));
}

/*------------------------- display_line() ------------------------------*/
/* This procedure displays a line of output given the results of a	 *
 * throughput test.  It displays the testcase name, the current priority *
 * level, the user/real time flag, and the elapsed time in seconds, as	 *
 * well as the percent change between the current and previous times.	 *
 * It then updates the previous elapsed time to be the current one.	 *
 *-----------------------------------------------------------------------*/
void display_line(char *tcname, int pri, int f, float et, float *pet, int ff)
{
	static int display_header = 0;
	float pc;		/* holds percent change */

	/*
	 * Print header every eight lines...
	 */
	if (display_header-- == 0) {
		printf("\n  Test                Processes              "
		       "  Time        Notes\n"
		       "---------   ---------------------------   "
		       "---------------  -------\n"
		       "name        long  short  priority  mode   "
		       "elapsed  %%%%delta\n\n");
		display_header = 6;
	}

	/*
	 * Calculate the percent change in time
	 */
	pc = (*pet == 0.0) ? 0.0 : 100.0 * ((et - *pet) / *pet) + 0.05;

	printf("%-12s %2d    %2d      %2d      %4s   %06.4f  %+06.4f  %s\n",
	       tcname, long_running, short_running, pri,
	       (f == 0) ? "user" : "real", et, pc, (ff) ? "forked child" : " ");

	fflush(stdout);

	*pet = et;
}

/*------------------------- perform_throughput_tests() ------------------*/
/* This procedure is called each time throughput tests are to be	 *
 * performed.  This procedure executes each of the throughput tests, and *
 * records the results of each to the .	 			 *
 *-----------------------------------------------------------------------*/
void perform_throughput_tests(int current_priority)
{
	float esecs,		/* elapsed seconds returned from each testcase */
	 esecs2,		/* elapsed seconds (second part) for sched_tc6 */
	 pc;			/* percent change for sched_tc6 */
	char pristr[10];	/* holds ascii value of priority as parameter */

	sprintf(pristr, "-p%d", current_priority);

#if defined(_IA64) && !defined(__64BIT__)
	esecs =
	    measure_test("sched_tc4.32", "-tvariable", pristr, NULL, &esecs2);
	display_line("sched_tc4.32", current_priority, 0, esecs, &e4user, 2);
	esecs = measure_test("sched_tc4.32", "-tfixed", pristr, NULL, &esecs2);
	display_line("sched_tc4.32", current_priority, 1, esecs, &e4real, 2);
	esecs =
	    measure_test("sched_tc5.32", "-tvariable", pristr, NULL, &esecs2);
	display_line("sched_tc5.32", current_priority, 0, esecs, &e5user, 2);
	esecs = measure_test("sched_tc5.32", "-tfixed", pristr, NULL, &esecs2);
	display_line("sched_tc5.32", current_priority, 1, esecs, &e5real, 2);
	esecs =
	    measure_test("sched_tc6.32", "-tvariable", pristr, " -d ", &esecs2);
	display_line("sched_tc6.32", current_priority, 0, esecs, &e6user0, 0);
	esecs =
	    measure_test("sched_tc6.32", "-tfixed", pristr, " -d ", &esecs2);
	display_line("sched_tc6.32", current_priority, 1, esecs, &e6real0, 0);
	esecs =
	    measure_test("sched_tc6.32", "-tvariable", pristr, " -df ",
			 &esecs2);
	display_line("sched_tc6.32", current_priority, 0, esecs, &e6user1, 1);
#else
	esecs = measure_test("sched_tc4", "-tvariable", pristr, NULL, &esecs2);
	display_line("sched_tc4", current_priority, 0, esecs, &e4user, 2);
	esecs = measure_test("sched_tc4", "-tfixed", pristr, NULL, &esecs2);
	display_line("sched_tc4", current_priority, 1, esecs, &e4real, 2);
	esecs = measure_test("sched_tc5", "-tvariable", pristr, NULL, &esecs2);
	display_line("sched_tc5", current_priority, 0, esecs, &e5user, 2);
	esecs = measure_test("sched_tc5", "-tfixed", pristr, NULL, &esecs2);
	display_line("sched_tc5", current_priority, 1, esecs, &e5real, 2);
	esecs =
	    measure_test("sched_tc6", "-tvariable", pristr, " -d ", &esecs2);
	display_line("sched_tc6", current_priority, 0, esecs, &e6user0, 0);
	esecs = measure_test("sched_tc6", "-tfixed", pristr, " -d ", &esecs2);
	display_line("sched_tc6", current_priority, 1, esecs, &e6real0, 0);
	esecs =
	    measure_test("sched_tc6", "-tvariable", pristr, " -df ", &esecs2);
	display_line("sched_tc6", current_priority, 0, esecs, &e6user1, 1);
#endif

	/*
	 * Manually build the display line for the second part of sched_tc6
	 */

	/*
	 * Calculate the percent change in time
	 */
	pc = (e6child ==
	      0.0) ? 0.0 : 100 * ((esecs2 - e6child) / e6child) + 0.05;
	printf("%-12s forked child          %4s   %06.4f  %+06.4f\n",
	       "sched_tc6", "real", esecs2, pc);
	e6child = esecs2;
}

/*------------------------ start_long_term_testcases() ------------------*/
/* This procedure takes the number of long-term process slots available, *
 * and executes the long term testcases.				 *
 *-----------------------------------------------------------------------*/
void start_long_term_testcases(long_term_slot_total, execution_time)
int long_term_slot_total;	/* total number of long-term slots */
char *execution_time;		/* runtime hours to pass to each testcase */
{
	int i;

	/*
	 * Now use up the long_term_slot_total by starting testcases call
	 * half with real-time flag '1' set, other half user flag '0'
	 */
	if (debug)
		printf("long-term slots available:  %d\n",
		       long_term_slot_total);

	for (i = 0; i < (long_term_slot_total / 4); i++) {
#if defined(_IA64) && !defined(__64BIT__)
		start_testcase("./sched_tc0.32", "sched_tc0 -t", execution_time,
			       " -p1", NULL, NULL);
		start_testcase("./sched_tc2.32", "sched_tc2", execution_time,
			       "1", NULL, NULL);
		start_testcase("./sched_tc0.32", "sched_tc0 -t", execution_time,
			       " -p0", NULL, NULL);
		start_testcase("./sched_tc2.32", "sched_tc2", execution_time,
			       "0", NULL, NULL);
#else
		start_testcase("./sched_tc0", "sched_tc0 -t", execution_time,
			       " -p1", NULL, NULL);
		start_testcase("./sched_tc2", "sched_tc2", execution_time, "1",
			       NULL, NULL);
		start_testcase("./sched_tc0", "sched_tc0 -t", execution_time,
			       " -p0", NULL, NULL);
		start_testcase("./sched_tc2", "sched_tc2", execution_time, "0",
			       NULL, NULL);
#endif
	}
}

/*---------------------------------------------------------------------+
|                     start_short_term_testcases ()                    |
| ==================================================================== |
|                                                                      |
| Function:  Starts short term testcases (one for each process slot)   |
|                                                                      |
+---------------------------------------------------------------------*/
void start_short_term_testcases(int short_term_slot_total,
				double stress_percent, int pri)
{
	int i;
	int short_term_slots;	/* number of slots to use */

	/*
	 * Set up the short_term_slot_total by starting testcases call
	 * half with real-time flag '1' set, other half user flag '0'
	 */
	if (available_user_process_slots() < short_term_slot_total)
		short_term_slots =
		    available_user_process_slots() * stress_percent / 2;
	else
		short_term_slots = short_term_slot_total;

	printf("\n<< Starting %d short-term testcases>> \n\n",
	       short_term_slots);
	if (debug)
		printf("short-term slots available:  %d\n", short_term_slots);

	for (i = 0; i < (short_term_slots / 4); i++) {
#if defined(_IA64) && !defined(__64BIT__)
		start_testcase("./sched_tc1.32", "sched_tc1", "1", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc3.32", "sched_tc3", "1", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc1.32", "sched_tc1", "0", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc3.32", "sched_tc3", "0", NULL, NULL,
			       NULL);
#else
		start_testcase("./sched_tc1", "sched_tc1", "1", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc3", "sched_tc3", "1", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc1", "sched_tc1", "0", NULL, NULL,
			       NULL);
		start_testcase("./sched_tc3", "sched_tc3", "0", NULL, NULL,
			       NULL);
#endif
#if 0
		perform_throughput_tests(pri);
#endif
	}
}

/*------------------------ kill_short_term_testcases() ------------------*/
/* This procedure goes through the process id table, and sends each	 *
 * process id number found in the table a signal in order to terminate	 *
 * it.  The signal sent is SIGUSR1.  It also re-initializes the table.	 *
 *-----------------------------------------------------------------------*/
void kill_short_term_testcases()
{
	int i;			/* loop counter to step through the list of process id's */

	/*
	 * Loop through the array of process id's one at a time, and
	 * attempt to kill each one.  If kill fails, report error and
	 * continue.
	 */
	if (debug)
		printf("killing short-term processes...\n");
	for (i = 0; i < numprocs; i++) {
		if (debug)
			printf("killing process [%d]\n", procs[i]);
		kill(procs[i], SIGUSR1);
	}

	/*
	 * Adjust the number of short_term_testcases
	 */
	short_running -= numprocs;

	/*
	 * Clear the table by setting number of entries to zero
	 */
	numprocs = 0;
}

/*----------------------------- finishup() ------------------------------*/
/* This procedure closing information to the about ending	 *
 * times, elapsed times, etc.  This procedure then closes the file*
 *-----------------------------------------------------------------------*/
void finishup(start_time)
long start_time;		/* starting time to calculate elapsed time */
{
	time_t end_time;		/* time when program finished */

	/*
	 * Get the end time and calculate elapsed time; write all this out
	 */
	end_time = time(NULL);

	printf("\nend time = %s\n", ctime(&end_time));

	printf("elapsed time = %4.2f hours\n",
	       ((end_time - start_time) / 3600.0));
}

/*---------------------------------------------------------------------+
|                             parse_args ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
| Updates:   (command line options)                                    |
|                                                                      |
|            [-s] size: shared memory segment size                     |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args(int argc, char **argv)
{
	int opt;
	int sflg = 0, pflg = 0, tflg = 0;
	int errflag = 0;
	char *program_name = *argv;
	extern char *optarg;	/* Command line option */

	/*
	 * Parse command line options.
	 */
	while ((opt = getopt(argc, argv, "vs:p:t:l:d")) != EOF) {
		switch (opt) {
		case 's':	/* stress percentage */
			sflg++;
			stress_percent = atof(optarg);
			break;
		case 'p':	/* process slots */
			pflg++;
			process_slots = atof(optarg);
			break;
		case 't':	/* time (hours) */
			tflg++;
			execution_time = atof(optarg);
			break;
		case 'd':	/* Enable debugging messages */
			debug++;
			break;
		case 'v':	/* Enable verbose mode=debug mode */
			debug++;
			break;
		default:
			errflag++;
			break;
		}
	}

	/*
	 * Check percentage, execution time and process slots...
	 */
	if (sflg) {
		if (stress_percent < 0.0 || stress_percent > 1.0)
			errflag++;
	}
	if (pflg) {
		if (process_slots < 0 || process_slots > MAXPROCS)
			errflag++;
	}
	if (tflg) {
		if (execution_time < 0.0 || execution_time > 100.0)
			errflag++;
	}
	if (debug)
		printf("\n(debugging messages enabled)\n\n");
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}
