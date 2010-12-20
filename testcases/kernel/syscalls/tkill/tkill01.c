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
/* File:	tkill01.c					   */
/*									    */
/* Description: This tests the tkill() syscall		      */
/*									    */
/* Usage:  <for command-line>						 */
/* tkill01 [-c n] [-e][-i n] [-I x] [-p x] [-t]		     */
/*      where,  -c n : Run n copies concurrently.			     */
/*	      -e   : Turn on errno logging.				 */
/*	      -i n : Execute test n times.				  */
/*	      -I x : Execute test for x seconds.			    */
/*	      -P x : Pause for x seconds between iterations.		*/
/*	      -t   : Turn on syscall timing.				*/
/*									    */
/* Total Tests: 1							     */
/*									    */
/* Test Name:   tkill01					     */
/* History:     Porting from Crackerjack to LTP is done by		    */
/*	      Manas Kumar Nayak maknayak@in.ibm.com>			*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <signal.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <sys/types.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "tkill01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 2;		   /* total number of tests in this file.   */

void cleanup() {

	TEST_CLEANUP;
	tst_rmdir();
}

void setup() {
	TEST_PAUSE;
	tst_tmpdir();
}

int sig_count = 0;

void sig_action(int sig)
{
	sig_count = 1;
}

int main(int ac, char **av) {
	int tid;
	int lc;		 /* loop counter */
	char *msg;	      /* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			if (signal(SIGUSR1, &sig_action) == SIG_ERR)
				tst_brkm(TBROK|TERRNO, cleanup,
				    "signal(SIGUSR1, ..) failed");
			TEST(tid = syscall( __NR_gettid));
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO, "tkill failed");
			}
			TEST(syscall(__NR_tkill,tid, SIGUSR1));
			if (TEST_RETURN == 0) {
				tst_resm(TPASS, "tkill call succeeded");
			} else {
				tst_resm(TFAIL|TTERRNO, "tkill failed");
			}
		}
	}
	cleanup();
	tst_exit();
}