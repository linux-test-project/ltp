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
/* File:        rt_sigaction01.c                                              */
/*                                                                            */
/* Description: This tests the rt_sigaction() syscall                         */
/*		rt_sigaction alters an action taken by a process on receipt   */
/* 		of a particular signal. The action is specified by the        */
/*		sigaction structure. The previous action on the signal is     */
/*		saved in oact.sigsetsize should indicate the size of a        */
/*		sigset_t type.                       			      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* rt_sigaction01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                        */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rt_sigaction01                                             */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigaction01";  /* Test program identifier.*/
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

int test_flags[] = {SA_RESETHAND|SA_SIGINFO, SA_RESETHAND, SA_RESETHAND|SA_SIGINFO, SA_RESETHAND|SA_SIGINFO, SA_NOMASK};

void
handler(int sig)
{
        tst_resm(TINFO,"Here is signal handler. Got signal: %d, do nothing here\n", sig);
        return;
}

int
set_handler(int sig, int sig_to_mask, int flag, int mask_flags)
{
        struct sigaction sa, oldaction;
        int err = 0;

        if (flag == 0) {
                tst_resm(TINFO,"flag0 - ");
                sa.sa_sigaction = (void *)handler;
                sa.sa_flags = mask_flags;
                TEST(sigemptyset(&sa.sa_mask));
                TEST(sigaddset(&sa.sa_mask, sig_to_mask));
                TEST(err = syscall(__NR_rt_sigaction,sig, &sa, NULL,8));
        } else if (flag == 1) {
                tst_resm(TINFO,"flag1 - ");
                TEST(err = syscall(__NR_rt_sigaction,sig, (void *)-1, NULL,8));
        } else if (flag == 2) {
               	tst_resm(TINFO,"flag2 - ");
                TEST(err = syscall(__NR_rt_sigaction,sig, NULL, (void *)-1,8));
	} else if (flag == 3) {
                tst_resm(TINFO,"flag3 - ");
                sa.sa_sigaction = (void *)handler;
                sa.sa_flags = mask_flags;
                TEST(sigemptyset(&sa.sa_mask));
                TEST(sigaddset(&sa.sa_mask, sig_to_mask));
                TEST(err = syscall(__NR_rt_sigaction,sig, &sa, &oldaction, 8));
                if (TEST_RETURN == 0) {
                        return 0;
                } else {
                        return TEST_ERRNO;
                }
        } else if (flag == 4){
                        TEST(err = syscall(__NR_rt_sigaction,sig, &sa, NULL,9));
        }
        if (TEST_RETURN == 0) {
                return 0;
        } else {
                return TEST_ERRNO;
        }
}


int main(int ac, char **av) {
	int retnval, i, j;
        int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */
	
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
                
			for (i = 0; i <= (SIGRTMAX + 1); i++){//signal for 0 to 65 
	    			tst_resm(TINFO,"Signal : %d",i);
			                for (j = 0; j < 4; j++){
			                       	 TEST(retnval = set_handler(i, 0, j, test_flags[j]));
						 if (TEST_RETURN == 0) {
        						tst_resm(TPASS, "rt_sigaction call succeeded: result = %d",TEST_RETURN);
			                         } else {
                 	   				tst_resm(TFAIL, "%s failed - errno = %d : %s", TCID, TEST_ERRNO, strerror(TEST_ERRNO));
							cleanup();
							tst_exit();
                       				}
                			}
        		}



                }
        }	
        tst_exit();
}

