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
/* File:        rt_sigprocmask02.c                                              */
/*                                                                            */
/* Description: This tests the rt_sigprocmask() syscall                       */
/*		rt_sigprocmask changes the list of currently blocked signals. */
/*		The set value stores the signal mask of the pending signals.  */
/*		The previous action on the signal is saved in oact. The value */
/*		of how indicates how the call should behave; its values are   */
/*		as follows:						      */
/*									      */
/*		SIG_BLOCK						      */
/*		    The set of blocked signals is the union of the current set*/ 
/*		    and the set argument. 				      */
/*		SIG_UNBLOCK						      */
/*		    The signals in set are removed from the current set of    */
/*		    blocked signals. It is okay to unblock a signal that is   */
/*		    not blocked. 					      */
/*		SIG_SETMASK						      */
/*		    The set of blocked signals is set to the set argument.    */
/*		    sigsetsize should indicate the size of a sigset_t type.   */
/* 									      */	
/* 		RETURN VALUE:i						      */
/* 		rt_sigprocmask returns 0 on success; otherwise, rt_sigprocmask*/ 
/* 		returns one of the errors listed in the "Errors" section.     */
/* 									      */
/* 		Errors:							      */
/* 			-EINVAL						      */	
/* 			    sigsetsize was not equivalent to the size of a    */
/* 			    sigset_t type or the value specified in how was   */
/* 			    invalid. 					      */
/* 			-EFAULT						      */
/* 			    An invalid set, act, or oact was specified.       */
/*                                                                            */
/*									      */
/* Usage:  <for command-line>                                                 */
/* rt_sigprocmask02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                      */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rt_sigprocmask02                                              */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"
#include "ltp_signal.h"

/* Extern Global Variables */
extern int Tst_count;		/* counter for tst_xxx routines.         */
extern char *TESTDIR;		/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigprocmask02";/* Test program identifier.*/
int  TST_TOTAL = 2;		/* total number of tests in this file.   */

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

sigset_t set;

struct test_case_t {
	sigset_t *ss;
	int sssize;
	int exp_errno;
} test_cases[] = {
	{ &set, 1, EINVAL },
	{ (sigset_t *)-1, SIGSETSIZE, EFAULT }
};

int test_count = sizeof(test_cases) / sizeof(struct test_case_t);

int main(int ac, char **av) {
	int i;
	sigset_t s;
	char *msg;	/* message returned from parse_opts */
	
	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	Tst_count = 0;

	TEST(sigfillset(&s));
	if (TEST_RETURN == -1){
		tst_resm(TFAIL | TTERRNO,
			"Call to sigfillset() failed.");
		cleanup();
		tst_exit();
	}

	for(i=0; i < test_count; i++) {
		TEST(syscall(__NR_rt_sigprocmask, SIG_BLOCK,
				&s, test_cases[i].ss,
				test_cases[i].sssize));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL | TTERRNO,
				"Call to rt_sigprocmask() succeeded, "
				"but should failed");
		} else if (TEST_ERRNO == test_cases[i].exp_errno) {
			tst_resm(TPASS | TTERRNO, "Got expected errno");
		} else {
			tst_resm(TFAIL | TTERRNO, "Got unexpected errno");
		}

	}

	cleanup();
	tst_exit();

}
