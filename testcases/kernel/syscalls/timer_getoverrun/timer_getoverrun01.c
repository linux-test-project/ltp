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
/* File:        timer_getoverrun01.c                                           */
/*                                                                            */
/* Description: This tests the timer_getoverrun() syscall                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* timer_getoverrun01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   timer_getoverrun01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <syscall.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* timer_t in kernel(int) is different from  Glibc definition(void*).
 * Use the kernel definition.
 */
typedef int kernel_timer_t;

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "timer_getoverrun01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 1;                   /* total number of tests in this file.   */

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

char tmpname[40];
int parent;
int block = 1;

#define ENTER(normal) tst_resm(TINFO, "Enter block %d: test %d (%s)", \
                         block, Tst_count, normal?"NORMAL":"ERROR");

int main(int ac, char **av) {
        int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */
	kernel_timer_t created_timer_id;
        struct sigevent ev;
	
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
                     
		ev.sigev_value = (sigval_t)0;
                ev.sigev_signo = SIGALRM;
                ev.sigev_notify = SIGEV_SIGNAL;

                TEST(syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &created_timer_id ));
                
		ENTER(1);
                TEST( syscall(__NR_timer_getoverrun, created_timer_id ));
		if(TEST_RETURN == 0){
			tst_resm(TPASS, "Block %d: test %d PASSED", block, Tst_count );
                } else {
			tst_resm(TFAIL, "Block %d: test %d FAILED... errno = %d : %s", block, Tst_count,TEST_ERRNO, strerror(TEST_ERRNO) );
                        cleanup();
                        tst_exit();
                }


		ENTER(0);
                TEST( syscall(__NR_timer_getoverrun, -1 ));
                if(TEST_RETURN < 0 && TEST_ERRNO == EINVAL ) {
                        tst_resm(TPASS, "Block %d: test %d PASSED", block, Tst_count );
                } else {
                        tst_resm(TFAIL, "Block %d: test %d FAILED... errno = %d : %s", block, Tst_count,TEST_ERRNO, strerror(TEST_ERRNO) );
                        cleanup();
                        tst_exit();
                }


                }
        }	
	cleanup();
        tst_exit();
}


/*

NAME
       timer_getoverrun
SYNOPSIS
       #include <time.h>

       int timer_getoverrun(timer_t timerid);

	Only a single signal shall be queued to the process for a given  timer
       at any point in time. When a timer for which a signal is still pending
       expires, no signal shall be queued, and a timer overrun  shall  occur.
        When  a timer expiration signal is delivered to or accepted by a pro-
       cess, if the implementation supports the Realtime  Signals  Extension,
       the  timer_getoverrun()  function  shall  return  the timer expiration
       overrun count for the specified timer. The overrun count returned con-
       tains  the number of extra timer expirations that occurred between the
       time the signal was generated (queued) and when it  was  delivered  or
       accepted, up to but not including an implementation-defined maximum of
       {DELAYTIMER_MAX}. If the number of such extra expirations  is  greater
       than or equal to {DELAYTIMER_MAX}, then the overrun count shall be set
       to {DELAYTIMER_MAX}. The value returned  by  timer_getoverrun()  shall
       apply  to the most recent expiration signal delivery or acceptance for
       the timer.  If no expiration signal has been delivered for the  timer,
       or  if  the  Realtime  Signals  Extension is not supported, the return
       value of timer_getoverrun() is unspecified.

RETURN VALUE
       If the timer_getoverrun() function succeeds, it shall return the timer
       expiration overrun count as explained above.
*/
