/******************************************************************************/
/* Copyright (c) Crackerjack Project., 2007				      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by	      */
/* the Free Software Foundation; either version 2 of the License, or	      */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		      */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									      */
/******************************************************************************/
/******************************************************************************/
/*									      */
/* File:		quotactl01.c					      */
/*									      */
/* Description: This tests the quotactl() syscall			      */
/*									      */
/* Usage:  <for command-line>						      */
/* quotactl01 [-c n] [-e][-i n] [-I x] [-p x] [-t]			      */
/*	  where,  -c n : Run n copies concurrently.			      */
/*			  -e   : Turn on errno logging.			      */
/*			  -i n : Execute test n times.			      */
/*			  -I x : Execute test for x seconds.		      */
/*			  -P x : Pause for x seconds between iterations.      */
/*			  -t   : Turn on syscall timing.		      */
/*									      */
/* Total Tests: 1							      */
/*									      */
/* Test Name:   quotactl01						      */
/* History:	 Porting from Crackerjack to LTP is done by		      */
/*			  Manas Kumar Nayak maknayak@in.ibm.com>	      */
/******************************************************************************/
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/types.h>
#include "config.h"
#if defined(HAS_QUOTAV2)
#define _LINUX_QUOTA_VERSION 2
#include <sys/quota.h>
/*
 * See m4/ltp-quota.m4 about the quota v1 vs quota v2 item.
#elif defined(HAS_RHEL_QUOTAV1)
#include <linux/quota.h>
 */
#else
/* Not HAS_QUOTAV2 */
#define BROKEN_QUOTACTL 1
#endif
/*
 * The broken manpage on my corporation's version of RHEL 4.6 says that these
 * headers are required (for quota v1), but I can't verify this requirement
 * because RHEL 4.6 doesn't RUN 2.4.x.
 *
 * Stale documentation ftl.
 * 
#if defined(HAS_RHEL_QUOTAV1)
#include <xfs/xqm.h>
#include <linux/dqblk_v1.h>
#include <linux/dqblk_v2.h>
#endif
 */

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int Tst_count;		   	/* counter for tst_xxx routines.	*/
extern char *TESTDIR;		  	/* temporary dir created by tst_tmpdir()*/

/* Global Variables */
char *TCID = "quotactl01";		/* Test program identifier.		*/
int  testno;
int  TST_TOTAL = 1;			/* total number of tests in this file.	*/

/* Extern Global Functions */
/******************************************************************************/
/*									      */
/* Function:	cleanup							      */
/*									      */
/* Description: Performs all one time clean up for this test on successful    */
/*		completion,  premature exit or  failure. Closes all temporary */
/*		files, removes all temporary directories exits the test with  */
/*		appropriate return code by calling tst_exit() function.	      */
/*									      */
/* Input:	None.							      */
/*									      */
/* Output:	None.							      */
/*									      */
/* Return:	On failure - Exits calling tst_exit(). Non '0' return code.   */
/*		On success - Exits calling tst_exit(). With '0' return code.  */
/*									      */
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
/*									      */
/* Function:	setup							      */
/*									      */
/* Description: Performs all one time setup for this test. This function is   */
/*		typically used to capture signals, create temporary dirs      */
/*		and temporary files that may be used in the course of this    */
/*		test.							      */
/*									      */
/* Input:	None.							      */
/*									      */
/* Output:	None.							      */
/*									      */
/* Return:	On failure - Exits by calling cleanup().		      */
/*		On success - returns 0.					      */
/*									      */
/******************************************************************************/
void setup() {
	/* Capture signals if any */
	/* Create temporary directories */
	if (geteuid() != 0) {
		tst_brkm(TCONF, tst_exit,
			"You must be root in order to execute this test");
	}
	TEST_PAUSE;
	tst_tmpdir();
}

/*
*  WARNING!! This test may cause the potential harm to the system, we DO NOT
*  provide any warranty for the safety!!
*/
/*
* To use this testcase, the quota function must be turned on and the user must
* be the super user.
*/

#ifdef BROKEN_QUOTACTL
int
main(void) {
	tst_resm(TBROK, "This system doesn't support quota v2");
	tst_exit();
}
#else
int cmd[] = {
	Q_QUOTAON,
	Q_QUOTAOFF,
	Q_GETQUOTA,
	Q_SETQUOTA,
/* Only available in quota v2 */
#if defined(HAS_QUOTAV2)
	Q_GETINFO,
	Q_SETINFO,
	Q_GETFMT,
#endif
	Q_SYNC
};

int
main(int ac, char **av)
{
	int id = getuid();
	int newtid = -1;
	int result;
	int ret;
	int i;
	int lc;				 /* loop counter */
	char *msg;			  /* message returned from parse_opts */
	
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

			for (i = 0; i <= 7; i++){

				ret = syscall(__NR_quotactl, cmd[i],
						(const char *)NULL, id,
						(caddr_t)NULL);
				if (ret != 0) {
					tst_resm(TFAIL|TTERRNO, "cmd=0x%x", cmd[i]);
				} else {
					tst_resm(TPASS, "quotactl call succeeded");
				}

			}

			TEST(result = syscall(__NR_set_tid_address, &newtid));

			if (TEST_RETURN == getpid()) {
				cleanup();
			} else {
				cleanup();
				tst_exit();
			}

		}

	}	

	cleanup();

	tst_exit();

}
#endif
