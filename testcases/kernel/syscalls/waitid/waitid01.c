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
/* File:        waitid01.c                                                    */
/*                                                                            */
/* Description: This tests the waitid() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* waitid01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                              */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   waitid01                                                      */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "waitid01";  /* Test program identifier.*/
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

void display_status(siginfo_t *infop)
{
        tst_resm(TINFO,"Process %d terminated:", infop->si_pid);
        tst_resm(TINFO,"code = %d",infop->si_code);
        if (infop->si_code == CLD_EXITED)
                tst_resm(TINFO,"exit value = %d",infop->si_status);
        else
		tst_resm(TINFO,"signal = %d",infop->si_status);
}

int main(int ac, char **av) {
	id_t pid;
        siginfo_t infop;
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

	TEST(fork());
	if (TEST_RETURN == 0) {
                exit(123);
        }
        else{
                TEST(waitid(P_ALL,getpid(),&infop,WEXITED));
		if (TEST_RETURN == -1) {
                        tst_resm(TFAIL|TTERRNO, "waitid(getpid()) failed");
                        tst_exit();
		}else
		    display_status(&infop); //CLD_EXITED = 1
        }

        TEST(fork());
        if (TEST_RETURN == 0) {
		int a, b = 0;
                a = 1/b;
                tst_exit();
        } else{
                TEST(waitid(P_ALL,0,&infop,WEXITED));
		if (TEST_RETURN == -1) {
                        tst_resm(TFAIL|TTERRNO, "waitid(0) failed");
                        tst_exit();
                } else
			display_status(&infop); //CLD_DUMPED = 3 ; SIGFPE = 8
        }

        TEST(pid = fork());
	if (TEST_RETURN == 0) {
                TEST(sleep(10));
                tst_exit();
        }
        TEST(kill(pid,SIGHUP));
        TEST(waitid(P_ALL,0,&infop,WEXITED));
	if (TEST_RETURN == -1) {
                tst_resm(TFAIL|TTERRNO, "waitid(0) failed");
                tst_exit();
        } else
		display_status(&infop); //CLD_KILLED = 2 ; SIGHUP = 1
                }
        }
        tst_resm(TPASS, "waitid(): system call passed");
	cleanup();
        tst_exit();
}