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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
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

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "tkill01";
int testno;
int TST_TOTAL = 2;

void cleanup(void)
{

	tst_rmdir();
}

void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int sig_count = 0;

void sig_action(int sig)
{
	sig_count = 1;
}

int main(int ac, char **av)
{
	int tid;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			if (signal(SIGUSR1, &sig_action) == SIG_ERR)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "signal(SIGUSR1, ..) failed");
			TEST(tid = ltp_syscall(__NR_gettid));
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO, "tkill failed");
			}
			TEST(ltp_syscall(__NR_tkill, tid, SIGUSR1));
			if (TEST_RETURN == 0) {
				tst_resm(TPASS, "tkill call succeeded");
			} else {
				tst_resm(TFAIL | TTERRNO, "tkill failed");
			}
		}
	}
	cleanup();
	tst_exit();
}
