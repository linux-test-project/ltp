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
/* File:        bdflush01.c                                            */
/*                                                                            */
/* Description: bdflush() starts, flushes, or tunes the buffer-dirty-flush    */
/*		daemon. Only a privileged process (one with the CAP_SYS_ADMIN */
/*		capability) may call bdflush().				      */
/*									      */
/*		If func is negative or 0, and no daemon has been started,     */
/*	        then bdflush() enters the daemon code and never returns.      */	
/*									      */	
/*		If func is 1, some dirty buffers are written to disk.	      */
/*		If func is 2 or more and is even (low bit is 0), then address */
/*		is the address of a long word, and the tuning parameter       */
/*		numbered (func-2)/2 is returned to the caller in that address.*/
/*									      */	
/*		If func is 3 or more and is odd (low bit is 1), then data is  */ 
/*		a long word, and the kernel sets tuning parameter numbered    */
/*		(func-3)/2 to that value.				      */
/*		    							      */
/*		The set of parameters, their values, and their legal ranges   */
/*		are defined in the kernel source file fs/buffer.c. 	      */
/*									      */
/*		Return Value:						      */
/*		If func is negative or 0 and the daemon successfully starts,  */
/*		bdflush() never returns. Otherwise, the return value is 0 on  */
/*		success and -1 on failure, with errno set to indicate the     */
/*		error.							      */	
/*									      */	
/*		Errors:							      */
/*			EBUSY						      */
/*			    An attempt was made to enter the daemon code after*/ 
/*			    another process has already entered. 	      */
/*			EFAULT						      */
/*			   address points outside your accessible address     */
/*			   space. 					      */	
/*			EINVAL						      */
/*			    An attempt was made to read or write an invalid   */
/*			    parameter number, or to write an invalid value to */
/*			    a parameter. 				      */
/*			EPERM						      */		
/*			    Caller does not have the CAP_SYS_ADMIN capability.*/
/*									      */
/* Usage:  <for command-line>                                                 */
/* bdflush01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                      */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   bdflush01                                              */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/kdaemon.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "bdflush01";  /* Test program identifier.*/
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

int main(int ac, char **av) {
	long data;
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
			TEST(syscall(__NR_bdflush,0,data));	//bdflush(0,data);
			if(TEST_RETURN < 0){
				tst_resm(TFAIL|TTERRNO,"Call to bdflush() failed");
                        	cleanup();
				tst_exit();
			}else {
				tst_resm(TPASS,"Test PASSED and %ld is returned \n",TEST_RETURN);
				tst_resm(TINFO,"bdflush() activated...\n");
                        	cleanup();
				tst_exit();
		        }

	
                }
	Tst_count++;
        }	
        tst_exit();
}

