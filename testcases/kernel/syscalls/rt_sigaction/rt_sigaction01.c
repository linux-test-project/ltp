/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007				   */
/*									    */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or	  */
/* (at your option) any later version.					*/
/*									    */
/* This program is distributed in the hope that it will be useful,	    */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		  */
/* the GNU General Public License for more details.			   */
/*									    */
/* You should have received a copy of the GNU General Public License	  */
/* along with this program;  if not, write to the Free Software	       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									    */
/******************************************************************************/
/******************************************************************************/
/*									    */
/* File:	rt_sigaction01.c					      */
/*									    */
/* Description: This tests the rt_sigaction() syscall			 */
/*		rt_sigaction alters an action taken by a process on receipt   */
/* 		of a particular signal. The action is specified by the	*/
/*		sigaction structure. The previous action on the signal is     */
/*		saved in oact.sigsetsize should indicate the size of a	*/
/*		sigset_t type.		       			      */
/*									    */
/* Usage:  <for command-line>						 */
/* rt_sigaction01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			*/
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   rt_sigaction01					     */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include "config.h"

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"
#include "ltp_signal.h"

/* Extern Global Variables */
extern int Tst_count;	   /* counter for tst_xxx routines.	 */
extern char *TESTDIR;	   /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigaction01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;		   /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*									    */
/* Function:    cleanup						       */
/*									    */
/* Description: Performs all one time clean up for this test on successful    */
/*	      completion,  premature exit or  failure. Closes all temporary */
/*	      files, removes all temporary directories exits the test with  */
/*	      appropriate return code by calling tst_exit() function.       */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*	      On success - Exits calling tst_exit(). With '0' return code.  */
/*									    */
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
/*									    */
/* Function:    setup							 */
/*									    */
/* Description: Performs all one time setup for this test. This function is   */
/*	      typically used to capture signals, create temporary dirs      */
/*	      and temporary files that may be used in the course of this    */
/*	      test.							 */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits by calling cleanup().		      */
/*	      On success - returns 0.				       */
/*									    */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int test_flags[] = {
	SA_RESETHAND|SA_SIGINFO,
	SA_RESETHAND,
	SA_RESETHAND|SA_SIGINFO,
	SA_RESETHAND|SA_SIGINFO,
	SA_NOMASK
};
char *test_flags_list[] = {
	"SA_RESETHAND|SA_SIGINFO",
	"SA_RESETHAND",
	"SA_RESETHAND|SA_SIGINFO",
	"SA_RESETHAND|SA_SIGINFO",
	"SA_NOMASK"
};

void
handler(int sig)
{
	tst_resm(TINFO, "Signal Handler Called with signal number %d", sig);
	return;
}

void
restorer(void) {

}

int
set_handler(int sig, int mask_flags)
{
	int rc = -1;
	struct sigaction sa, oldaction;

	memset(&sa, 0, SIGSETSIZE);

	sa.sa_sigaction = (void *)handler;
	sa.sa_flags = mask_flags;

	if (sigemptyset(&sa.sa_mask) < 0) {
		tst_resm(TINFO, "sigemptyset(..) failed");
	} else if (sigaddset(&sa.sa_mask, sig) < 0) {
		tst_resm(TFAIL | TINFO, "sigaddset(..) failed");
	} else if (syscall(__NR_rt_sigaction, sig, &sa, &oldaction, SIGSETSIZE)) {
		tst_resm(TFAIL | TERRNO, "rt_sigaction(%d, ..) failed", sig);
	} else {
		rc = 0;
	}
	return rc;
}

int main(int ac, char **av) {
	int signal, flag;
	int lc;		/* loop counter */
	char *msg;	/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			/* 34 (NPTL) or 35 (LinuxThreads) to 65 (or 128 on mips). */
			for (signal = SIGRTMIN; signal <= SIGRTMAX; signal++) { 

				tst_resm(TINFO, "signal: %d ", signal);

			 	for (flag = 0; flag < sizeof(test_flags) / sizeof(int); flag++) {

					if (set_handler(signal, test_flags[flag]) == 0) {
						tst_resm(TINFO,
							"sa.sa_flags = %s",
							test_flags_list[flag]);
						kill(getpid(), signal);
		       			}

				}
	
			}



		}
	}	

	cleanup();
	tst_exit();

}
