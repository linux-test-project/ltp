/*
 *
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
 *  FILE        : pth_str02.c
 *  DESCRIPTION : Create n threads
 *  HISTORY:
 *    05/16/2001 Paul Larson (plars@us.ibm.com)
 *      -Ported
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "test.h"

/* Defines
 *
 * DEFAULT_NUM_THREADS: Default number of threads to create,
 * user can specifiy with [-n] command line option.
 *
 * USAGE: usage statement
 */

#define DEFAULT_NUM_THREADS 		10
#define USAGE	"\nUsage: %s [-l | -n num_threads] [-d]\n\n" \
		"\t-l		     Test as many as threads as possible\n" \
		"\t-n num_threads    Number of threads to create\n" \
		"\t-d                Debug option\n\n"

/*
 * Function prototypes
 *
 * sys_error (): System error message function
 * error (): Error message function
 * parse_args (): Parses command line arguments
 */

static void sys_error(const char *, int);
static void error(const char *, int);
static void parse_args(int, char **);
void *thread(void *);

/*
 * Global Variables
 */

int num_threads = DEFAULT_NUM_THREADS;
int test_limit = 0;
int debug = 0;

char *TCID = "pth_str02";
int TST_TOTAL = 1;

/*---------------------------------------------------------------------+
|                               main ()                                |
| ==================================================================== |
|                                                                      |
| Function:  Main program  (see prolog for more details)               |
|                                                                      |
+---------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	/*
	 * Parse command line arguments and print out program header
	 */
	parse_args(argc, argv);

	if (test_limit) {
		tst_resm(TINFO, "Creating as many threads as possible");
	} else {
		tst_resm(TINFO, "Creating %d threads", num_threads);
	}
	thread(0);

	/*
	 * Program completed successfully...
	 */
	tst_resm(TPASS, "Test passed");
	exit(0);
}

/*---------------------------------------------------------------------+
|                               thread ()                              |
| ==================================================================== |
|                                                                      |
| Function:  Recursively creates threads while num < num_threads       |
|                                                                      |
+---------------------------------------------------------------------*/
void *thread(void *parm)
{
	intptr_t num = (intptr_t) parm;
	pthread_t th;
	pthread_attr_t attr;
	size_t stacksize = 1046528;
	int pcrterr;

	/*
	 * Create threads while num < num_threads...
	 */
	if (test_limit || (num < num_threads)) {

		if (pthread_attr_init(&attr))
			sys_error("pthread_attr_init failed", __LINE__);
		if (pthread_attr_setstacksize(&attr, stacksize))
			sys_error("pthread_attr_setstacksize failed", __LINE__);
		if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE))
			sys_error("pthread_attr_setdetachstate failed",
				  __LINE__);
		/************************************************/
		/*   pthread_create does not touch errno.  It RETURNS the error
		 *   if it fails.  errno has no bearing on this test, so it was
		 *   removed and replaced with return value check(see man page
		 *   for pthread_create();
		 */
		pcrterr = pthread_create(&th, &attr, thread, (void *)(num + 1));
		if (pcrterr != 0) {
			if (test_limit) {
				tst_resm(TINFO,
					 "Testing pthread limit, %d pthreads created.",
					 (int)num);
				pthread_exit(0);
			}
			if (pcrterr == EAGAIN) {
				tst_resm(TINFO,
					 "Thread [%d]: unable to create more threads!",
					 (int)num);
				return NULL;
			} else
				sys_error("pthread_create failed", __LINE__);
		}
		pthread_join(th, NULL);
	}

	return 0;
	/*
	   pthread_exit(0);
	 */
}

/*---------------------------------------------------------------------+
|                             parse_args ()                            |
| ==================================================================== |
|                                                                      |
| Function:  Parse the command line arguments & initialize global      |
|            variables.                                                |
|                                                                      |
+---------------------------------------------------------------------*/
static void parse_args(int argc, char **argv)
{
	int i;
	int errflag = 0;
	char *program_name = *argv;

	while ((i = getopt(argc, argv, "dln:?")) != EOF) {
		switch (i) {
		case 'd':	/* debug option */
			debug++;
			break;
		case 'l':	/* test pthread limit */
			test_limit++;
			break;
		case 'n':	/* number of threads */
			num_threads = atoi(optarg);
			break;
		case '?':
			errflag++;
			break;
		}
	}

	/* If any errors exit program */
	if (errflag) {
		fprintf(stderr, USAGE, program_name);
		exit(2);
	}
}

/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
static void sys_error(const char *msg, int line)
{
	char syserr_msg[256];

	sprintf(syserr_msg, "%s: %s\n", msg, strerror(errno));
	error(syserr_msg, line);
}

/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
static void error(const char *msg, int line)
{
	fprintf(stderr, "ERROR [line: %d] %s\n", line, msg);
	tst_resm(TFAIL, "Test failed");
	exit(-1);
}
