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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/types.h>
#include "config.h"
#if defined(HAS_QUOTAV2) || defined(HAS_QUOTAV1)
#if defined(HAS_QUOTAV2)
#define _LINUX_QUOTA_VERSION 2
#else	/* HAS_QUOTAV1 */
#define _LINUX_QUOTA_VERSION 1
#endif
#include <sys/quota.h>
#else	/* ! (HAS_QUOTAV2 || HAS_QUOTAV1) */
/* Not HAS_QUOTAV2 */
#define BROKEN_QUOTACTL 1
#endif

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

#define QUOTACTL(cmd, addr) \
	syscall(__NR_quotactl, QCMD(cmd, USRQUOTA), block_dev, id, \
					(caddr_t) addr)
#ifndef BROKEN_QUOTACTL

#ifndef QUOTAFILE
/* Default name of the quota file in Fedora 12. */
#define QUOTAFILE "aquota.user"
#endif

char quota_started = 0;
static char *block_dev, *mountpoint, *quota_file, *quota_loc = NULL;
int id;
struct dqblk dq;

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
extern void cleanup()
{

	/* Remove tmp dir and all files in it */
	TEST_CLEANUP;
	tst_rmdir();

	if (block_dev) {
		if (quota_started == 1 && QUOTACTL(Q_QUOTAOFF, &dq)) {
			tst_brkm(TBROK | TERRNO, NULL,
				"failed to disable the quota on %s", block_dev);
		}
	}

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
	if ((quota_loc = malloc(FILENAME_MAX)) == NULL) {
		tst_brkm(TCONF | TERRNO, tst_exit,
			"couldn't allocate memory for the quota loc buffer");
	}

	TEST_PAUSE;
	tst_tmpdir();

	snprintf(quota_loc, FILENAME_MAX, "%s/%s", mountpoint, quota_file);

	if (QUOTACTL(Q_QUOTAON, quota_loc) != 0) {
		
		if (errno == ENOENT) {
			tst_brkm(TCONF, cleanup,
				"quota file - %s - doesn't exist (is the name "
				"correct?)", quota_loc);
		} else {
			/* Provide a terse explanation for why the command
			 * failed.. */
			tst_brkm(TCONF | TERRNO, cleanup,
				"failed to enable quotas on block device: %s; "
				"1. Ensure that the device is mounted with the "
				"quota option. 2. Check the filesystem status "
				"with `quotacheck %s'", block_dev, block_dev);
		}
	} else {
		quota_started = 1;
	}

}
#endif

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

	static int block_dev_FLAG = 0, mountpoint_FLAG = 0, quota_file_FLAG = 0;
	option_t opts[] = {
		{ .option = "b:", .flag = &block_dev_FLAG, .arg = &block_dev },
		{ .option = "m:", .flag = &mountpoint_FLAG, .arg = &mountpoint },
		{ .option = "q:", .flag = &quota_file_FLAG, .arg = &quota_file },
		{ .option = '\0' }
	};

	int newtid = -1;
	int result;
	int ret;
	int i;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t*) opts, NULL)) != (char*)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		Tst_count = 0;

		for (testno = 0; testno < TST_TOTAL; ++testno) {

			for (i = 0; i <= sizeof(cmd)/sizeof(cmd[0]); i++){

				ret = QUOTACTL(cmd[i], &dq);
				if (ret != 0) {
					tst_resm(TFAIL | TERRNO,
						"cmd=0x%x failed", cmd[i]);
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
