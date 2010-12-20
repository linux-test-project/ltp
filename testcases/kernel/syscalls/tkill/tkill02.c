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
/* File:	tkill02.c					   */
/*									    */
/* Description: This tests the tkill() syscall		      */
/*									    */
/* Usage:  <for command-line>						 */
/* tkill02 [-c n] [-e][-i n] [-I x] [-p x] [-t]		     */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   tkill02					     */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "tkill02";  /* Test program identifier.*/
int  testno;

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

	TEST_CLEANUP;
	tst_rmdir();
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

struct test_case_t {
	int tid;
	int exp_errno;
} test_cases[] = {
	{ -1, EINVAL },
	{ 99999, ESRCH }
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av) {
	int i;

	setup();

	for (i = 0; i < TST_TOTAL; i++) {

		TEST(syscall(__NR_tkill, test_cases[i].tid, SIGUSR1));

		if (TEST_RETURN == -1) {
		    	if (TEST_ERRNO == test_cases[i].exp_errno) {
				tst_resm(TPASS|TTERRNO,
				    "tkill(%d, SIGUSR1) failed as expected",
				    test_cases[i].tid);
			} else {
				tst_brkm(TFAIL|TTERRNO, cleanup,
				    "tkill(%d, SIGUSR1) failed unexpectedly",
				    test_cases[i].tid);
			}
		} else {
			tst_brkm(TFAIL, cleanup,
			    "tkill(%d) succeeded unexpectedly",
			    test_cases[i].tid);
		}
	}
	cleanup();
	tst_exit();
}