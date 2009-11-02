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
/* File:        rt_sigaction02.c                                              */
/*                                                                            */
/* Description: This tests the rt_sigaction() syscall                         */
/*		rt_sigaction Expected EFAULT error check                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* rt_sigaction02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                        */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   rt_sigaction02                                             */
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

/*
 * For all but __mips__:
 *
 * _COMPAT_NSIG / _COMPAT_NSIG_BPW == 2.
 *
 * For __mips__:
 *
 * _COMPAT_NSIG / _COMPAT_NSIG_BPW == 4.
 *
 * See asm/compat.h under the kernel source for more details.
 *
 * Multiply that by a fudge factor of 4 and you have your SIGSETSIZE.
 */
#if defined (__mips__)
#define SIGSETSIZE 16
#else
#define SIGSETSIZE 8
#endif

#define INVAL_STRUCT -1

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "rt_sigaction02";  /* Test program identifier.*/
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
char *test_flags_list[] = {"SA_RESETHAND|SA_SIGINFO", "SA_RESETHAND", "SA_RESETHAND|SA_SIGINFO", "SA_RESETHAND|SA_SIGINFO", "SA_NOMASK"};

struct test_case_t {
        int exp_errno;
        char *errdesc;
} test_cases[] = {
	{ EFAULT, "EFAULT" }
};


int main(int ac, char **av) {
	int signal, flag;
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
                
			for (signal = SIGRTMIN; signal <= (SIGRTMAX ); signal++){//signal for 34 to 65 
			 	for(flag=0; flag<5;flag++) {

				/*   							        *
				 * long sys_rt_sigaction (int sig, const struct sigaction *act, *
				 * truct sigaction *oact, size_t sigsetsize);			*
				 * EFAULT:							*
		        	 * An invalid act or oact value was specified 			*
				 */

					 TEST(syscall(__NR_rt_sigaction,signal, INVAL_STRUCT, NULL,SIGSETSIZE));
					if((TEST_RETURN == -1) && (TEST_ERRNO == test_cases[0].exp_errno)) {
        						tst_resm(TINFO, "sa.sa_flags = %s ",test_flags_list[flag]);
                 	   				tst_resm(TPASS, "%s failure with sig: %d as expected errno  = %s : %s", TCID, signal,test_cases[0].errdesc, strerror(TEST_ERRNO));
			                         } else {
        					tst_resm(TFAIL, "rt_sigaction call succeeded: result = %ld got error %d:but expected  %d", TEST_RETURN, TEST_ERRNO, test_cases[0].exp_errno);
        					tst_resm(TINFO, "sa.sa_flags = %s ",test_flags_list[flag]);
						}
                			}
		 	printf("\n");	
        		}



                }
        }	
	cleanup();
        tst_exit();
}

