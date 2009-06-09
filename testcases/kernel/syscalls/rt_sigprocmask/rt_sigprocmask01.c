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
/* File:        rt_sigprocmask01.c                                              */
/*                                                                            */
/* Description: This tests the rt_sigprocmask() syscall                       */
/*		rt_sigprocmask changes the list of currently blocked signals. */
/*		The set value stores the signal mask of the pending signals.  */
/*		The previous action on the signal is saved in oact. The value */
/*		of how indicates how the call should behave; its values are   */
/*		as follows:						      */
/*									      */
/*		SIG_BLOCK						      */
/*		    The set of blocked signals is the union of the current set*/ 
/*		    and the set argument. 				      */
/*		SIG_UNBLOCK						      */
/*		    The signals in set are removed from the current set of    */
/*		    blocked signals. It is okay to unblock a signal that is   */
/*		    not blocked. 					      */
/*		SIG_SETMASK						      */
/*		    The set of blocked signals is set to the set argument.    */
/*		    sigsetsize should indicate the size of a sigset_t type.   */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* rt_sigprocmask01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                      */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rt_sigprocmask01                                              */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigprocmask01";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 8;                   /* total number of tests in this file.   */

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

int sig_count = 0;

void sig_handler(int sig)
{
        sig_count++;
}

int main(int ac, char **av) {
	struct sigaction act, oact;
        sigset_t set, oset;
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
			TEST(sigemptyset(&set));
			if(TEST_RETURN == -1){
				tst_resm(TFAIL,"Call to sigemptyset() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        	cleanup();
				tst_exit();
			}
			TEST(sigaddset(&set, 33));
			if(TEST_RETURN == -1){
				tst_resm(TFAIL,"Call to sigaddset() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        	cleanup();
				tst_exit();
			}
			
			/* call rt_sigaction() */
			act.sa_handler = sig_handler;
                        TEST(syscall(__NR_rt_sigaction, 33, &act, &oact, 8));
			if(TEST_RETURN != 0){
				tst_resm(TFAIL,"Call to rt_sigaction() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        	cleanup();
				tst_exit();
			}
			/* call rt_sigprocmask() to block signal#33 */
                        TEST(syscall(__NR_rt_sigprocmask, SIG_BLOCK, &set, &oset, 8));
			if(TEST_RETURN == -1){
				tst_resm(TFAIL,"Call to rt_sigprocmask()**** Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        	cleanup();
				tst_exit();
			}
			
			else {
				TEST(kill(getpid(), 33));
				if(TEST_RETURN != 0){
					tst_resm(TFAIL,"Call to kill() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        		cleanup();
					tst_exit();
					
				}
				if(sig_count){
					tst_resm(TFAIL,"FAIL --- rt_sigprocmask() fail to change the process's signal mask, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        		cleanup();
					tst_exit();
				}
				else {
					/* call rt_sigpending() */
					TEST(syscall(__NR_rt_sigpending, &oset, 8));
					if(TEST_RETURN == -1){
						tst_resm(TFAIL,"call rt_sigpending() failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        			cleanup();
						tst_exit();
					}
					TEST(sigismember(&oset, 33));
					if(TEST_RETURN == 0 ){
						tst_resm(TFAIL,"call sigismember() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        			cleanup();
						tst_exit();
					}
					/* call rt_sigprocmask() to unblock signal#33 */
					TEST(syscall(__NR_rt_sigprocmask, SIG_UNBLOCK, &set, &oset, 8));
					if(TEST_RETURN == -1){
						tst_resm(TFAIL,"Call to rt_sigprocmask() Failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
                        			cleanup();
						tst_exit();
					}
					if(sig_count) {
						tst_resm(TPASS,"rt_sigprocmask() PASSED");
                        			cleanup();
						tst_exit();
					}
		                        else {
						tst_resm(TFAIL,"rt_sigprocmask() functionality failed, errno=%d : %s",TEST_ERRNO, strerror(TEST_ERRNO));
		                                cleanup();
						tst_exit();
		                        }

				}
			}

                }
	Tst_count++;
        }	
        tst_exit();
}

