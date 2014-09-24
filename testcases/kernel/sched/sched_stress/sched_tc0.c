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
|                               sched_tc0                              |
| ==================================================================== |
|                                                                      |
| Description:  Creates long-term disk I/O bound process               |
|                                                                      |
| Algorithm:    o  Set process priority                                |
|               o  Open a large file and repeatedly read from it       |
|                  for a specified time duration                       |
|                                                                      |
| To compile:   cc -o sched_tc0 sched_tc0.c -L. -lpsc                  |
|                                                                      |
| Usage:        sched_tc0 [-t sec] [-p priority] [-v] [-d]             |
|                                                                      |
| Last update:   Ver. 1.3, 4/10/94 23:04:59                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     010402  Manoj Iyer Ported to Linux			       |
|                                                                      |
+---------------------------------------------------------------------*/

#include   <stdlib.h>
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
 * DEFAULT_EXECUTION_TIME: default execution time (in seconds)
 *
 */
#define DEFAULT_PRIORITY_TYPE	"variable"
#define DEFAULT_EXECUTION_TIME	1800
#define USAGE "Usage:  %s  [-p priority] [-t sec] [-v] [-d]      \n" \
              "        -t sec      execution time (default 1800 sec)    \n" \
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

/*
 * Global variables:
 *
 * verbose: enable normal messages
 *
 * debug: enable debugging messages
 *
 * execution_time: testcase execution time (hours)
 *
 * priority: process type (fixed priority, variable priority)
 */
int verbose = 0;
int debug = 0;
long execution_time = DEFAULT_EXECUTION_TIME;
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
	long start_time;	/* time at start of testcase */
	int i;

	if ((filename = getenv("KERNEL")) == NULL) {
		errno = ENODATA;
		sys_error("environment variable KERNEL not set", __FILE__,
			  __LINE__);
	}
	/*
	 * Process command line arguments...
	 */

	parse_args(argc, argv);
	if (verbose)
		printf("%s: Scheduler TestSuite program\n\n", *argv);
	if (debug) {
		printf("\tpriority:       %s\n", priority);
		printf("\texecution_time: %ld (sec)\n", execution_time);
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
	 * Continuously read through file as time permits...
	 */
	i = 0;
	start_time = time(NULL);

	if (debug)
		printf("\n");
	while ((time(NULL) - start_time) < execution_time) {
		if (debug) {
			printf("\r\tprocessing file [%d], time left: %ld",
			       i++,
			       execution_time - (time(NULL) - start_time));
			fflush(stdout);
		}
		process_file(filename);
	}
	if (debug)
		printf("\n");

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
	int pflg = 0, tflg = 0;
	int errflag = 0;
	char *program_name = *argv;
	extern char *optarg;	/* Command line option */

	if (argc < 2) {
		fprintf(stderr, USAGE, program_name);
		exit(0);
	}

	/*
	 * Parse command line options.
	 */
	while ((opt = getopt(argc, argv, "p:t:vd")) != EOF) {
		switch (opt) {
		case 'p':	/* process type */
			pflg++;
			priority = optarg;
			break;
		case 't':	/* time (hours) */
			tflg++;
			execution_time = atof(optarg);
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
	if (tflg) {
		if (execution_time < 0.0 || execution_time > 360000)
			errflag++;
	}
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}
