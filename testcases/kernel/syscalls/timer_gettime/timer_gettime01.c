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
/* File:        timer_gettime01.c                                           */
/*                                                                            */
/* Description: This tests the timer_gettime() syscall                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* timer_gettime01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   timer_gettime01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <time.h>
#include <signal.h>
#include <syscall.h>
#include <stdio.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* timer_t in kernel(int) is different from  Glibc definition(void*).
 * Use the kernel definition.
 */
typedef int kernel_timer_t;

/* Extern Global Variables */

/* Global Variables */
char *TCID = "timer_gettime01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 3;                   /* total number of tests in this file.   */

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

	TEST_CLEANUP;
	tst_rmdir();

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

#define ENTER(normal) tst_resm(TINFO, "Enter block %d: test %d (%s)", \
		 block, Tst_count, normal?"NORMAL":"ERROR");

char tmpname[40];
int parent;
int block = 1;

int main(int ac, char **av) {
	int lc;                 /* loop counter */
	char *msg;              /* message returned from parse_opts */

	kernel_timer_t created_timer_id;
	struct sigevent ev;
	struct itimerspec spec;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			ENTER(1);
			ev.sigev_value = (sigval_t)0;
			ev.sigev_signo = SIGALRM;
			ev.sigev_notify = SIGEV_SIGNAL;
			TEST(syscall(__NR_timer_create, CLOCK_REALTIME, &ev,
				&created_timer_id));
			TEST(syscall(__NR_timer_gettime, created_timer_id,
				&spec));
			if (TEST_RETURN == 0) {
				tst_resm(TPASS, "Block %d: test %d PASSED",
					block, Tst_count);
			} else {
				tst_brkm(TFAIL|TERRNO, cleanup,
				    "Block %d: test %d FAILED",
				    block, Tst_count);
			}

/*

ERRORS
       -EINVAL
	      An invalid timer_id value was specified.
*/
			ENTER(0);
			TEST(syscall(__NR_timer_gettime, -1, &spec));
			if (TEST_RETURN < 0 && TEST_ERRNO == EINVAL) {
				tst_resm(TPASS,
				    "Block %d: test %d PASSED",
				    block, Tst_count);
			} else {
				tst_brkm(TFAIL|TERRNO, cleanup,
				    "Block %d: test %d FAILED",
				    block, Tst_count);
			}

/*

       -EFAULT
	      An invalid address of setting was specified.
*/

			ENTER(0);
			TEST(syscall(__NR_timer_gettime, created_timer_id,
			    NULL));
			if (TEST_RETURN < 0 && TEST_ERRNO == EFAULT ) {
				tst_resm(TPASS,
				    "Block %d: test %d PASSED",
				    block, Tst_count);
			} else {
				tst_brkm(TFAIL|TERRNO, cleanup,
				    "Block %d: test %d FAILED",
				    block, Tst_count);
			}

		}
	}

	cleanup();
	tst_exit();

}