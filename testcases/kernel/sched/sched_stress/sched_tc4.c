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
/*
 * FUNCTIONS: Scheduler Test Suite
 */

/*---------------------------------------------------------------------+
|                               sched_tc4                              |
| ==================================================================== |
|                                                                      |
| Description:  Creates short-term disk I/O bound process              |
|                                                                      |
| Algorithm:    o  Set process priority                                |
|               o  Continuously multiply matrices together until       |
|                  interrupted.                                        |
|                                                                      |
| To compile:   cc -o sched_tc4 sched_tc4.c -L. -lpsc                  |
|                                                                      |
| Usage:        sched_tc4 [-t priority_type] [-p priority]             |
|                         [-l log] [-v] [-d]                           |
|                                                                      |
| Last update:   Ver. 1.3, 4/10/94 23:05:02                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     050689  CTU   Initial version                             |
|    0.2     010402  Manoj Iyer Ported to Linux			       |
|                                                                      |
+---------------------------------------------------------------------*/

#include   <sys/times.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include   <stdlib.h>
#include   "sched.h"

/*
 * Defines:
 *
 * USAGE: usage statement
 *
 * DEFAULT_PRIORITY_TYPE: default priority
 *
 * BLOCK_SIZE: block size (in bytes) for raw I/O
 *
 * TIMES: number of times to read raw I/O device (~25MB)
 *
 */
#define DEFAULT_PRIORITY_TYPE	"variable"
#define DEFAULT_LOGFILE		"sched_tc4.log"
#define BLOCK_SIZE		512
#define TIMES			5000
#define USAGE "Usage:  %s  [-l log] [-t type] [-p priority] [-v] [-d]\n" \
              "        -l log      log file                             \n" \
              "        -t type     priority type 'variable' or 'fixed'  \n" \
              "        -p priority priority value                       \n" \
              "        -v          verbose                              \n" \
              "        -d          enable debugging messages            \n"

/*
 * Function prototypes:
 *
 * process_file: reads data file
 *
 * parse_args: parse command line arguments
 */
void parse_args(int, char **);
void read_raw_device();

/*
 * Global variables:
 *
 * verbose: enable normal messages
 *
 * debug: enable debugging messages
 *
 * priority: process type (fixed priority, variable priority)
 */
int verbose = 0;
int debug = 0;
int priority = DEFAULT_PRIORITY;
char *logfile = DEFAULT_LOGFILE;
char *priority_type = DEFAULT_PRIORITY_TYPE;

/*---------------------------------------------------------------------+
|                                 main                                 |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	FILE *statfile;
	clock_t start_time;	/* start & stop times */
	clock_t stop_time;
	float elapsed_time;
	struct tms timer_info;	/* time accounting info */

	/*
	 * Process command line arguments...
	 */
	parse_args(argc, argv);
	if (verbose)
		printf("%s: Scheduler TestSuite program\n\n", *argv);
	if (debug) {
		printf("\tpriority type:  %s\n", priority_type);
		printf("\tpriority:       %d\n", priority);
		printf("\tlogfile:        %s\n", logfile);
	}

	/*
	 * Adjust the priority of this process if the real time flag is set
	 */
	if (!strcmp(priority_type, "fixed")) {
#ifndef __linux__
		if (setpri(0, DEFAULT_PRIORITY) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#else
		if (setpriority(PRIO_PROCESS, 0, 0) < 0)
			sys_error("setpri failed", __FILE__, __LINE__);
#endif
	} else {
		if (nice((priority - 50) - (nice(0) + 20)) < 0 && errno != 0)
			sys_error("nice failed", __FILE__, __LINE__);
	}

	/*
	 * Read from raw I/O device and record elapsed time...
	 */
	start_time = time((time_t *) & timer_info);

	read_raw_device();

	stop_time = time((time_t *) & timer_info);
	elapsed_time = (float)(stop_time - start_time) / 100.0;

	if ((statfile = fopen(logfile, "w")) == NULL)
		sys_error("fopen failed", __FILE__, __LINE__);

	fprintf(statfile, "%f\n", elapsed_time);
	if (debug)
		printf("\n\telapsed time: %f\n", elapsed_time);

	if (fclose(statfile) < 0)
		sys_error("fclose failed", __FILE__, __LINE__);

	/*
	 * Exit with success!
	 */
	if (verbose)
		printf("\nsuccessful!\n");
	return (0);
}

/*---------------------------------------------------------------------+
|                          read_raw_device ()                          |
| ==================================================================== |
|                                                                      |
| Function:  o  opens raw disk device                                  |
|            o  reads block of size BLOCK_SIZE n times                 |
|            o  lseeks back to beginning of file                       |
|            o  closes raw device                                      |
|                                                                      |
+---------------------------------------------------------------------*/
void read_raw_device()
{
	char readbuf[BLOCK_SIZE + 1];	/* buffer to store bytes read */
	int fd;			/* file descriptor */
	int i;			/* loop counter */
	int blocks = 0;		/* number of blocks read */
#ifndef __linux__
	char raw_dev[50] = "/dev/hd2";	/* name of raw device file */
#else
	char *raw_dev;		/* name of raw device file  */

	if ((raw_dev = getenv("RAWDEV")) == NULL) {
		errno = ENODATA;
		sys_error("environment variable RAWDEV not set", __FILE__,
			  __LINE__);
	}
#endif

	/*
	 * Open the raw disk file
	 */
	if ((fd = open(raw_dev, 0)) < 0)
		sys_error("open failed", __FILE__, __LINE__);

	/*
	 * Read through predefined number of blocks TIMES number of times.
	 * (Seek back to beginning of raw device after reading 10MB)
	 */
	for (i = 0; i < TIMES; i++) {
		if (read(fd, readbuf, BLOCK_SIZE) != BLOCK_SIZE)
			sys_error("read failed", __FILE__, __LINE__);
		if (blocks == 10000)
			if (lseek(fd, 0, 0) < 0)
				sys_error("lseek failed", __FILE__, __LINE__);
	}

	/*
	 * Close the raw disk file
	 */
	if (close(fd) < 0)
		sys_error("close failed", __FILE__, __LINE__);
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
|            [-t] type:     priority type "fixed" or "variable"        |
|            [-p] priority: priority value                             |
|            [-l] logfile:  log file name                              |
|            [-v]           verbose                                    |
|            [-d]           enable debugging messages                  |
|                                                                      |
+---------------------------------------------------------------------*/
void parse_args(int argc, char **argv)
{
	int opt;
	int lflg = 0, pflg = 0, tflg = 0;
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

	while ((opt = getopt(argc, argv, "l:t:p:vd")) != EOF) {
		switch (opt) {
		case 'l':	/* log file */
			lflg++;
			logfile = optarg;
			break;
		case 't':	/* process type */
			tflg++;
			priority_type = optarg;
			break;
		case 'p':	/* process priority */
			pflg++;
			priority = atoi(optarg);
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
	 * Check percentage and process slots...
	 */
	if (tflg) {
		if (strcmp(priority_type, "fixed") &&
		    strcmp(priority_type, "variable")) {
			errflag++;
			fprintf(stderr, "Error: priority type must be: "
				"\'fixed\' or \'variable\'\n");
		}
	}
	if (pflg) {
		if (priority < 50 || priority > 100) {
			errflag++;
			fprintf(stderr, "Error: priority range [50..100]\n");
		}
	}
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}
