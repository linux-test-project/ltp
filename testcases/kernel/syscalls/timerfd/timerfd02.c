/******************************************************************************/
/*                                                                            */
/* Copyright (c) Ulrich Drepper <drepper@redhat.com>                          */
/* Copyright (c) International Business Machines  Corp., 2009                 */
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
/* File:        timerfd02.c                                                   */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=11fcb6c14676023d0bd437841f5dcd670e7990a0 */
/*              says:                                                         */
/* The timerfd_create syscall already has a flags parameter.  It just is      */
/* unused so far.  This patch changes this by introducing the TFD_CLOEXEC     */
/* flag to set the close-on-exec flag for the returned file descriptor. A new */
/* name TFD_CLOEXEC is introduced which in this implementation must have the  */
/* same value as O_CLOEXEC.                                                   */
/* The following test must be adjusted for architectures other than x86 and   */
/* x86-64 and in case the syscall numbers changed.                            */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* timerfd02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                           */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   timerfd02                                                     */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 08 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 08 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#ifndef O_CLOEXEC
# define O_CLOEXEC 02000000
#endif

#define TFD_CLOEXEC O_CLOEXEC

/* Extern Global Variables */
extern int Tst_count;		/* counter for tst_xxx routines.         */
extern char *TESTDIR;		/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "timerfd02";	/* test program identifier.              */
int testno;
int TST_TOTAL = 1;		/* total number of tests in this file.   */

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
extern void cleanup()
{
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
void setup()
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int argc, char *argv[])
{
	int fd, coe;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}
	if ((tst_kvercmp(2, 6, 27)) < 0) {
		tst_resm(TCONF,
			 "This test can only run on kernels that are 2.6.27 and higher");
		tst_exit();
	}
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			fd = syscall(__NR_timerfd_create, CLOCK_REALTIME, 0);
			if (fd == -1) {
				tst_resm(TFAIL, "timerfd_create(0) failed");
				cleanup();
				tst_exit();
			}
			coe = fcntl(fd, F_GETFD);
			if (coe == -1) {
				tst_brkm(TBROK, cleanup, "fcntl failed");
				tst_exit();
			}
			if (coe & FD_CLOEXEC) {
				tst_resm(TFAIL,
					 "timerfd_create(0) set close-on-exec flag");
				cleanup();
				tst_exit();
			}
			close(fd);

			fd = syscall(__NR_timerfd_create, CLOCK_REALTIME,
				     TFD_CLOEXEC);
			if (fd == -1) {
				tst_resm(TFAIL,
					 "timerfd_create(TFD_CLOEXEC) failed");
				cleanup();
				tst_exit();
			}
			coe = fcntl(fd, F_GETFD);
			if (coe == -1) {
				tst_brkm(TBROK, cleanup, "fcntl failed");
				tst_exit();
			}
			if ((coe & FD_CLOEXEC) == 0) {
				tst_resm(TFAIL,
					 "timerfd_create(TFD_CLOEXEC) set close-on-exec flag");
				cleanup();
				tst_exit();
			}
			close(fd);
			tst_resm(TPASS, "timerfd_create(TFD_CLOEXEC) Passed");
			cleanup();
		}
	}
	tst_exit();
}
