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

/* 11/05/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

			   /* page02.c */
/*======================================================================
	=================== TESTPLAN SEGMENT ===================
CALLS:	malloc(3)

	Run with KILL flag.

>KEYS:  < paging behavior
>WHAT:  < Does the system balk at heavy demands on it's paging facilities?
>HOW:   < Create a number of process, each of which requests a large
	< chunk of memory to be assigned to an array.  Write to each
	< element in that array, and verify that what was written/stored
	< is what was expected.
	  Writes start in middle of array and proceede to ends.
>BUGS:  <
======================================================================*/

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#ifdef LINUX
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#endif

/** LTP Port **/
#include "test.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;

char *TCID = "page02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

int bd_arg(char *);
int chld_flag;
int parent_pid;

int main(argc, argv)
int argc;
char *argv[];
{
	int nchild;
	int memory_size, half_memory_size;
	int error_count, i, j, pid, status;
	int *memory_pointer;
	int *up_pointer, *down_pointer;
	int child, count;
	int chld();

	parent_pid = getpid();
	tst_tmpdir();

	if (signal(SIGUSR1, (void (*)())chld) == SIG_ERR) {
		tst_resm(TBROK, "signal failed");
		exit(1);
	}

	if (argc < 2) {
		memory_size = 128 * 1024;
		nchild = 5;
	} else if (argc == 3) {
		if (sscanf(argv[1], "%d", &memory_size) != 1)
			bd_arg(argv[1]);
		if (sscanf(argv[2], "%d", &nchild) != 1)
			bd_arg(argv[2]);
	} else {
		printf("page02 [memory size (words)]  [nchild]\n");
		tst_resm(TCONF, "\tBad arg count.\n");
		exit(1);
	}
	half_memory_size = memory_size / 2;

	error_count = 0;

	/****************************************/
	/*                                      */
	/*      attempt to fork a number of     */
	/*      identical processes             */
	/*                                      */
	/****************************************/

	for (i = 1; i <= nchild; i++) {
		chld_flag = 0;
		if ((pid = fork()) == -1) {
			tst_resm(TBROK,
				 "Fork failed (may be OK if under stress)");
			tst_resm(TINFO, "System resource may be too low.\n");
			local_flag = PASSED;
			tst_brkm(TBROK, tst_rmdir, "Reason: %s\n",
				 strerror(errno));
		} else if (pid == 0) {
			/********************************/
			/*                              */
			/*   allocate memory  of size   */
			/*    "memory_size"             */
			/*                              */
			/********************************/

			memory_pointer = malloc(memory_size * sizeof(int));
			if (memory_pointer == 0) {
				tst_resm(TBROK, "\tCannot malloc memory.\n");
				if (i < 2) {
					tst_resm(TBROK,
						 "\tThis should not happen to first two children.\n");
					tst_resm(TBROK, "\tChild %d - fail.\n",
						 i);
				} else {
					tst_resm(TBROK,
						 "\tThis is ok for all but first two children.\n");
					tst_resm(TBROK, "\tChild %d - ok.\n",
						 i);
					kill(parent_pid, SIGUSR1);
					_exit(0);
				}
				tst_resm(TBROK, "malloc fail");
				tst_resm(TFAIL,
					 "\t\nImpossible to allocate memory of size %d in process %d\n",
					 memory_size, i);
				kill(parent_pid, SIGUSR1);
				tst_exit();
			}
			kill(parent_pid, SIGUSR1);

			down_pointer = up_pointer = memory_pointer +
			    (memory_size / 2);

			/********************************/
			/*                              */
			/*         write to it          */
			/*                              */
			/********************************/

			for (j = 1; j <= half_memory_size; j++) {
				*(up_pointer++) = j;
				*(down_pointer--) = j;
			}
			sleep(1);

			/********************************/
			/*                              */
			/*      and read from it to     */
			/*  check that what was written */
			/*       is still there         */
			/*                              */
			/********************************/

			down_pointer = up_pointer = memory_pointer +
			    (memory_size / 2);

			for (j = 1; j <= half_memory_size; j++) {
				if (*(up_pointer++) != j)
					error_count++;
				if (*(down_pointer--) != j)
					error_count++;
			}
			exit(error_count);
		}
		while (!chld_flag)
			sleep(1);
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
		tst_resm(TINFO, "\tTest {%d} exited status %d\n", child,
			 status);
#endif
		if (status)
			local_flag = FAILED;
		count++;
	}

	if (count != nchild) {
		tst_resm(TFAIL, "\tWrong number of children waited on.\n");
		tst_resm(TFAIL, "\tCount = %d, expected = %d.\n",
			 count, nchild);
	}

	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed")
	    : tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();

}

int bd_arg(str)
char *str;
{
	tst_brkm(TCONF, NULL, "\tCannot parse %s as a number.\n", str);
}

int chld()
{
	if (signal(SIGUSR1, (void (*)())chld) == SIG_ERR) {
		tst_brkm(TBROK, NULL, "signal failed");
	}
	chld_flag++;
	return 0;
}
