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
/* FUNCTIONS: Scheduler Test Suite */

/*---------------------------------------------------------------------+
|                               sched_tc1                              |
| ==================================================================== |
|                                                                      |
| Description:  Creates short-term disk I/O bound process              |
|                                                                      |
| Algorithm:    o  Set process priority                                |
|               o  Open a large file                                   |
|               o  Continuously read from file until interrupted       |
|               o  Close file upon receiving interrupt                 |
|                                                                      |
| To compile:   cc -o sched_tc1 sched_tc1.c -L. -lpsc                  |
|                                                                      |
| Usage:        sched_tc1 [-p priority] [-v] [-d]                      |
|                                                                      |
| Last update:   Ver. 1.4, 4/10/94 23:05:00                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     010402  Manoj Iyer Ported to Linux			       |
|                                                                      |
+---------------------------------------------------------------------*/

#include   <stdlib.h>
#include   <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include   "sched.h"

/*
 * Defines:
 *
 * USAGE: usage statement
 *
 * DEFAULT_PRIORITY_TYPE: default priority
 *
 */
#define DEFAULT_PRIORITY_TYPE	"variable"
#define USAGE "Usage:  %s  [-p priority] [-v] [-d]               \n" \
              "        -p priority priority (default variable)          \n" \
              "        -v          verbose                              \n" \
              "        -d          enable debugging messages            \n"

/*
 * Function prototypes:
 *
 * process_file: reads data file
 *
 * parse_args: parse command line arguments
 */
void process_file(char *);
void parse_args(int, char **);
void signal_handler();

/*
 * Global variables:
 *
 * verbose: enable normal messages
 *
 * debug: enable debugging messages
 *
 * signaled: set upon receiving SIGUSER1 signal
 *
 * priority: process type (fixed priority, variable priority)
 */
int verbose = 0;
int debug = 0;
int signaled = 0;
char *priority = DEFAULT_PRIORITY_TYPE;

/*---------------------------------------------------------------------+
|                                 main                                 |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	char *filename = NULL;

	if ((filename = getenv("KERNEL")) == NULL) {
		errno = ENODATA;
		sys_error("environment variable KERNEL not set", __FILE__,
			  __LINE__);
	}
	/*
	 * Setup signal handler & setup alarm so we do not loop forever...
	 */
	signal(SIGUSR1, signal_handler);
	signal(SIGALRM, signal_handler);
	alarm(600);		/* wait 10 minutes before aborting */

	/*
	 * Process command line arguments...
	 */
	parse_args(argc, argv);
	if (verbose)
		printf("%s: Scheduler TestSuite program\n\n", *argv);
	if (debug) {
		printf("\tpriority:       %s\n", priority);
	}

	/*
	 * Adjust the priority of this process if the real time flag is set
	 */
	if (!strcmp(priority, "fixed")) {
#ifndef __linux__
		if (setpri(0, DEFAULT_PRIORITY) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#else
		if (setpriority(PRIO_PROCESS, 0, 0) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#endif
	}

	/*
	 * Continuously read through file until interrupted...
	 */
	while (!signaled)
		process_file(filename);

	/*
	 * Exit with success!
	 */
	if (verbose)
		printf("\nsuccessful!\n");
	return (0);
}

/*---------------------------------------------------------------------+
|                            process_file ()                           |
| ==================================================================== |
|                                                                      |
| Function:  Opens a file, reads from it until it encounters an        |
|            end-of-file and then closes the file..                    |
|                                                                      |
+---------------------------------------------------------------------*/
void process_file(char *filename)
{
	char record[100];	/* holds each record of the file read */
	FILE *datafile;		/* file pointer to the open file */

	/*
	 * Try and open the datafile
	 */
	if ((datafile = fopen(filename, "r")) == NULL)
		sys_error("fopen failed", __FILE__, __LINE__);

	/*
	 * Read the first record of the datafile, then read until end-of-file
	 */
	while (fgets(record, 80, datafile)) {
		if (feof(datafile))
			break;
	}

	/*
	 * Close the datafile
	 */
	if (fclose(datafile))
		sys_error("fclose failed", __FILE__, __LINE__);
}

/*---------------------------------------------------------------------+
|                           signal_handler ()                          |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void signal_handler(int signal)
{
	printf("signal received is %d\n", signal);
	if (signal == SIGUSR1) {
		signaled++;
		if (debug)
			printf("\n\t<< caught SIGUSR1 interrupt>>\n");
	} else if (signal == SIGALRM) {
		error("Failed to receive SIGUSR1 signal before timeout!",
		      __FILE__, __LINE__);
	} else
		error("received unexpected signal", __FILE__, __LINE__);
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
|            [-p] priority: "fixed" or "variable"                      |
|            [-t] n:        execution time in hours                    |
|            [-v]           verbose                                    |
|            [-d]           enable debugging messages                  |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args(int argc, char **argv)
{
	int opt;
	int pflg = 0;
	int errflag = 0;
	char *program_name = *argv;
	extern char *optarg;	/* Command line option */

	/*
	 * Parse command line options.
	 */
	if (argc < 2) {
		fprintf(stderr, USAGE, program_name);
		exit(0);
	}

	while ((opt = getopt(argc, argv, "p:t:vd")) != EOF) {
		switch (opt) {
		case 'p':	/* process type */
			pflg++;
			priority = optarg;
			break;
		case 'v':	/* verbose */
			verbose++;
			break;
		case 'd':	/* enable debugging messages */
			verbose++;
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
	if (pflg) {
		if (strcmp(priority, "fixed") && strcmp(priority, "variable"))
			errflag++;
	}
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}
