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
/* File:        sgetmask01.c                                           */
/*                                                                            */
/* Description: This tests the sgetmask() syscall                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* sgetmask01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   sgetmask01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

/* NOTE:  This case test the behavior of sgetmask
# Sometime the returned "Oops"in this case don't mean anything for
# correct or error, we check the result between different kernel and
# try to find if there exist different returned code in different kernel
#
*/

#include <stdio.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "sgetmask01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 2;                   /* total number of tests in this file.   */

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
        int sig;
	int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */

        /* parse standard options */
        if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
             tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
             tst_exit();
           }

        setup();

        for (lc = 0; TEST_LOOPING(lc); ++lc) {
                Tst_count = 0;
                for (testno = 0; testno < TST_TOTAL; ++testno) {

			for (sig = -3; sig <= SIGRTMAX + 1; sig++) {
				TEST(syscall(__NR_ssetmask,sig));
                		tst_resm(TINFO,"Setting signal : %d -- return of setmask : %ld",sig,TEST_RETURN);     //call sgetmask()
                     		TEST(syscall(__NR_sgetmask));     //call sgetmask()
                     		if (TEST_RETURN != sig) {
        				tst_resm(TINFO,"Oops,setting sig %d, got %ld",sig,TEST_RETURN);
                     		} else
        				tst_resm(TPASS,"OK,setting sig %d, got %ld",sig,TEST_RETURN);
                     		if (sig == SIGRTMAX + 1) {
					cleanup();
					tst_exit();
				}
                	}
		}
        }
	cleanup();
        tst_exit();
}