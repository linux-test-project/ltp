/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007                                   */
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
/* File:        rt_sigqueueinfo01.c                                           */
/*                                                                            */
/* Description: This tests the rt_sigqueueinfo() syscall.                     */
/*		rt_sigqueueinfo() Send signal information to a signal	      */
/*									      */
/* Usage:  <for command-line>                                                 */
/* rt_sigqueueinfo01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 2                                                             */
/*                                                                            */
/* Test Name:   rt_sigqueueinfo01                                              */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count; /* counter for tst_xxx routines.         */
extern char *TESTDIR; /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigqueueinfo01"; /* Test program identifier.*/
int testno;
int TST_TOTAL = 2; /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup() {
	/* Remove tmp dir and all files in it */
	TEST_CLEANUP;
	tst_rmdir();

	/* Exit with appropriate return code. */
	tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av) {
	int pid, retval;
	pid = getpid();
	siginfo_t uinfo;
	int lc; /* loop counter */
	char *msg; /* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *) NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			TEST(pid = fork());
			setup();
			if (TEST_RETURN < 0) {
				tst_resm(TFAIL, "fork() Failed, errno=%d : %s", TEST_ERRNO,
						strerror(TEST_ERRNO));
				cleanup();
			} else if (TEST_RETURN == 0) {
				uinfo.si_errno = 0;
				uinfo.si_code = SI_QUEUE;
				TEST(retval = syscall(__NR_rt_sigqueueinfo, getpid(), 17,
						&uinfo));
				if (TEST_RETURN == 0) {
					tst_resm(TPASS, "Test Succeeded");
				} else {
					tst_resm(TFAIL, "Test Failed, errno=%d : %s", TEST_ERRNO,
							strerror(TEST_ERRNO));
				}
				cleanup();
			}
			tst_exit();
		}
		Tst_count++;
	}
}

