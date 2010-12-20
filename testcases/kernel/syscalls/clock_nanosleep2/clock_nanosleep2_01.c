/******************************************************************************/
/* Copyright (c) M. Koehrer <mathias_koehrer@arcor.de>, 2009                  */
/*                                                                            */
/* LKML Reference: http://lkml.org/lkml/2009/4/9/89                           */
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
/* File:        clock_nanosleep2_01.c                                         */
/*                                                                            */
/* Description: This tests the clock_nanosleep2-out() syscall                 */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* clock_nanosleep2_01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                   */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   clock_nanosleep2_01                                           */
/******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */

/* Global Variables */
char *TCID = "clock_nanosleep2_01";  /* Test program identifier.*/
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

const clockid_t CLOCK_TO_USE=CLOCK_MONOTONIC;
static int clock_nanosleep2(clockid_t clock_id, int flags, const struct timespec *req,struct timespec *rem)
{
	return syscall(__NR_clock_nanosleep, clock_id, flags, req, rem);
}

int main(int ac, char **av) {
	int i;
	int lc;                 /* loop counter */
	char *msg;              /* message returned from parse_opts */
	struct timespec ts;

        /* parse standard options */
        if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
             tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
             tst_exit();
           }

        setup();

        for (lc = 0; TEST_LOOPING(lc); ++lc) {
                Tst_count = 0;
                for (testno = 0; testno < TST_TOTAL; ++testno) {
			TEST(clock_gettime(CLOCK_TO_USE, &ts));
			for (i=0; i<=50; i++) {
				ts.tv_sec++;
				TEST(clock_nanosleep2(CLOCK_TO_USE, TIMER_ABSTIME, &ts, NULL));
                                if (TEST_ERRNO) {
                                    tst_resm(TFAIL, "%s failed - errno = %d : %s", TCID, TEST_ERRNO, strerror(TEST_ERRNO));
                                    cleanup();
                                    tst_exit();
                                }
				tst_resm(TINFO,"Iteration = %i",i);
			}
                        tst_resm(TPASS, "clock_nanosleep2() passed");
		}
        }
        tst_exit();
}