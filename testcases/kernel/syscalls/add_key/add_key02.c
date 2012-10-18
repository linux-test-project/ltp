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
/* File:	add_key02.c					   */
/*									    */
/* Description: This tests the add_key() syscall		      */
/*									    */
/* Usage:  <for command-line>						 */
/* add_key02 [-c n] [-e][-i n] [-I x] [-p x] [-t]		     */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   add_key02					     */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <linux/keyctl.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "add_key02";  /* Test program identifier.*/
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
	char *type;
	char *desc;
	void *payload;
	int plen;
	int exp_errno;
} test_cases[] = {
	{ "user", "firstkey", NULL, 1, EINVAL }
};

int test_count = sizeof(test_cases) / sizeof(struct test_case_t);

int main(int ac, char **av) {
	int i;
	int lc;		 /* loop counter */
	char *msg;	      /* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			for (i = 0; i<test_count; i++) {

				/* Call add_key. */
				TEST(syscall(__NR_add_key, test_cases[i].type,
						test_cases[i].desc,
						test_cases[i].payload,
						test_cases[i].plen,
						KEY_SPEC_USER_KEYRING));

				if (TEST_RETURN != -1) {
					tst_resm(TINFO,
					    "add_key passed unexpectedly");
				} else {

					if (errno == test_cases[i].exp_errno) {
						tst_resm(TINFO|TTERRNO,
							"called add_key() "
							"with wrong args got "
							"EXPECTED errno");
					} else {
						tst_resm(TFAIL|TTERRNO,
							"called add_key() "
							"with wrong args got "
							"UNEXPECTED errno");
					}

				}

			}

		}

	}

	cleanup();
	tst_exit();
}
