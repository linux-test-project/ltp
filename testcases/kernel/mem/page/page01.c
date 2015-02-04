/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 11/01/2002	Port to LTP  	robbiew@us.ibm.com */

			   /* page01.c */
/*======================================================================
	=================== TESTPLAN SEGMENT ===================
CALLS:	malloc(3)

	Run with KILL flag

>KEYS:  < paging behavior
>WHAT:  < Does the system balk at heavy demands on it's paging facilities?
>HOW:   < Create a number of process, each of which requests a large
	< chunk of memory to be assigned to an array.  Write to each
	< element in that array, and verify that what was written/stored
	< is what was expected.
>BUGS:  <
======================================================================*/

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int bd_arg(char *);

/** LTP Port **/
#include "test.h"

void blenter(void);
void setup(void);
void anyfail(void);
void ok_exit(void);
void forkfail(void);
void terror(char *);
int instress(void);

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;
FILE *temp;

char *TCID = "page01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

int main(argc, argv)
int argc;
char *argv[];
{
	int nchild;
	int memory_size;
	int error_count, i, j, pid, status;
	int *number_pointer;
	int *memory_pointer;
	int child, count;

	setup();

	if (argc < 2) {
		memory_size = 256 * 1024;
		nchild = 50;
	} else if (argc == 3) {
		if (sscanf(argv[1], "%d", &memory_size) != 1)
			bd_arg(argv[1]);
		if (sscanf(argv[2], "%d", &nchild) != 1)
			bd_arg(argv[2]);
	} else {
		printf("page01 [memory size (words)]  [nchild]\n");
		tst_resm(TCONF, "\tBad arg count.\n");
		exit(1);
	}

	blenter();

	error_count = 0;

	/****************************************/
	/*                                      */
	/*      attempt to fork a number of     */
	/*      identical processes             */
	/*                                      */
	/****************************************/

	for (i = 1; i <= nchild; i++) {
		if ((pid = fork()) == -1) {
			terror("Fork failed (may be OK if under stress)");
			if (instress())
				ok_exit();
			forkfail();
		} else if (pid == 0) {
			/********************************/
			/*                              */
			/*   allocate memory  of size   */
			/*    "memory_size"             */
			/*                              */
			/********************************/

			memory_pointer = malloc(memory_size * sizeof(int));
			if (memory_pointer == 0) {
				tst_resm(TBROK,
					 "Cannot allocate memory - malloc failed.\n");
				if (i < 2) {
					tst_resm(TBROK,
						 "This should not happen for first two children.\n");
					tst_brkm(TFAIL, NULL,
						 "Child %d - fail.\n",
						 i);
				} else {
					tst_resm(TCONF,
						 "This is ok for all but first two children.\n");
					tst_brkm(TCONF, NULL,
						 "Child %d - ok.\n", i);
				}
			}
			number_pointer = memory_pointer;

			/********************************/
			/*                              */
			/*         write to it          */
			/*                              */
			/********************************/

			for (j = 1; j <= memory_size; j++)
				*(number_pointer++) = j;
			sleep(1);

			/********************************/
			/*                              */
			/*      and read from it to     */
			/*  check that what was written */
			/*       is still there         */
			/*                              */
			/********************************/

			number_pointer = memory_pointer;
			for (j = 1; j <= memory_size; j++) {
				if (*(number_pointer++) != j)
					error_count++;
			}
			exit(error_count);
		}
	}

	/****************************************/
	/*                                      */
	/*      wait for the child processes    */
	/*      to teminate and report the #    */
	/*      of deviations recognized        */
	/*                                      */
	/****************************************/

	count = 0;
	while ((child = wait(&status)) > 0) {
#ifdef DEBUG
		tst_resm(TINFO, "Test {%d} exited status %d\n", child, status);
#endif
		if (status)
			local_flag = FAILED;
		count++;
	}

	if (count != nchild) {
		tst_resm(TWARN, "Wrong number of children waited on.\n");
		tst_resm(TWARN, "Count = %d, expected = %d.\n", count, nchild);
	}

	anyfail();
	/** NOT REACHED **/
	tst_exit();
}

int bd_arg(str)
char *str;
{
	tst_resm(TCONF, "\tCannot parse %s as a number.\n", str);
	exit(1);
}

/** LTP Port **/
/*
 * setup
 *
 * Do set up - here its a dummy function
 */
void setup()
{
	tst_tmpdir();
	temp = stderr;
}

/*
 * Function: blenter()
 *
 * Description: Print message on entering a new block
 */
void blenter()
{
	local_flag = PASSED;
	return;
}

/*
 *
 * Function: anyfail()
 *
 * Description: Exit a test.
 */
void anyfail()
{
	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed")
	    : tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();
}

/*
 * ok_exit
 *
 * Calling block passed the test
 */
void ok_exit()
{
	local_flag = PASSED;
	return;
}

/*
 * forkfail()
 *
 * exit on failure
 */
void forkfail()
{
	tst_brkm(TBROK, tst_rmdir, "Reason: %s\n", strerror(errno));
}

/*
 * Function: terror
 *
 * Description: prints error message this may not be because some part of the
 *              test case failed, for example fork() failed. We will log this
 *              failure as TBROK instead of TFAIL.
 */
void terror(char *message)
{
	tst_resm(TBROK, "Reason: %s:%s\n", message, strerror(errno));
	return;
}

/*
 * instress
 *
 * Assume that we are always running under stress, so this function will
 * return > 0 value always.
 */
int instress()
{
	tst_resm(TINFO, "System resource may be too low, fork() malloc()"
		 " etc are likely to fail.\n");
	return 1;
}
