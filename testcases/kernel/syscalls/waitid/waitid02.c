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
/* File:        waitid02.c                                           	      */
/*                                                                            */
/* Description: This tests the waitid() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* waitid02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                              */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   waitid02                                                      */
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
extern int Tst_count;           /* counter for tst_xxx routines.         */
extern char *TESTDIR;           /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "waitid02";  /* Test program identifier.*/
int  testno;
int  TST_TOTAL = 7;                   /* total number of tests in this file.   */

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


int errnochoose(void){   //choose the relative errno

    switch (TEST_ERRNO){
        case    0:      tst_exit();
        case    ECHILD:  strerror((int)"ECHILD");
                tst_exit();
        case    EINTR:  strerror((int)"EINTR");
                tst_exit();
        case    EINVAL: strerror((int)"EINVAL");
                tst_exit();
        default:     strerror((int)"Other Error");
                tst_exit();
   }
        TEST_RETURN = 0;
        //tst_exit() ;
}


int main(int ac, char **av) {
	id_t cpid;
        id_t id1,id2,id3;
	id_t gid1,gid2,gid3;
        siginfo_t infop;
        int i = 0;

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
                     
	
	TEST(waitid(P_ALL,0,&infop,WNOHANG));
	if(TEST_RETURN == 0)
                tst_resm(TPASS,"Success !");
        else{
                tst_resm(TFAIL,"Error. is your system >2.6.9 ?");
                errnochoose();
        }

// option == WEXITED | WCONTINUED | WSTOPPED | WNOHANG | WNOWAIT ;

	TEST(fork());
	if(TEST_RETURN == 0){
                tst_resm(TINFO,"I'm a child,my id is %d,gpid is %d",id1=getpid(),gid1=getpgid(0));
		exit(5);
        }
        
	TEST(fork());
	if(TEST_RETURN == 0){
                tst_resm(TINFO,"I'm a child,my id is %d,gpid is %d",id2=getpid(),gid2=getpgid(0));
		exit(7);
        }

	TEST(fork());
	if(TEST_RETURN == 0){
                tst_resm(TINFO,"I'm a child,my id is %d,gpid is %d",id3=getpid(),gid3=getpgid(0));
		exit(6);
        }

        tst_resm(TINFO,"I'm a father %d",cpid=getpid());

        TEST(waitid(P_PGID,cpid,&infop,WEXITED));
	if(TEST_RETURN == 0){				
	 tst_resm(TPASS,"Success ... 0 is returned.");
         tst_resm(TINFO,"si_pid = %d ; si_code = %d ; si_status = %d",infop.si_pid,infop.si_code,infop.si_status);
	}else {
		tst_resm(TFAIL,"Fail...  %d is returned",TEST_RETURN);
                errnochoose();
	     }

        TEST(waitid(P_PGID,id2,&infop,WEXITED));
	if(TEST_RETURN == 0){				//NOCHILD
         tst_resm(TINFO,"si_pid = %d ; si_code = %d ; si_status = %d",infop.si_pid,infop.si_code,infop.si_status);
	 tst_resm(TPASS,"Success2 ... 0 is returned");
	}else {
		tst_resm(TFAIL,"Fail...  %d is returned",TEST_RETURN);
                errnochoose();
	     }

        TEST(i = waitid(P_PID,id1,&infop,WCONTINUED));
	if(TEST_RETURN == 0){				//EINVAL
         tst_resm(TINFO,"si_pid = %d ; si_code = %d ; si_status = %d",infop.si_pid,infop.si_code,infop.si_status);
	 tst_resm(TPASS,"Success3 ... 0 is returned");
	}else {
		tst_resm(TFAIL,"Fail...  %d is returned",i);
                errnochoose();
	     }
          }
     }	
        cleanup();
	tst_exit();
}



