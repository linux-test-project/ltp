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
/* File:        dup3_01.c                                                     */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=336dd1f70ff62d7dd8655228caed4c5bfc818c56 */
/*              says:                                                         */
/* This patch adds the new dup3 syscall.  It extends the old dup2 syscall by  */
/* one parameter which is meant to hold a flag value.  Support for the        */
/* O_CLOEXEC flag is added in this patch. The following test must be adjusted */
/* for architectures other than x86 and x86-64 and in case the                */
/* syscall numbers changed.                                                   */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* dup3_01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                               */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   dup3_01                                                       */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
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

/* Global Variables */
char *TCID = "dup3_01";		/* test program identifier.              */
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
void cleanup()
{
	TEST_CLEANUP;
	tst_rmdir();
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

	if ((tst_kvercmp(2, 6, 27)) < 0)
		tst_brkm(TCONF, NULL,
			 "This test can only run on kernels that are 2.6.27 and higher");
	setup();

	fd = syscall(__NR_dup3, 1, 4, 0);
	if (fd == -1) {
		tst_brkm(TFAIL|TERRNO, cleanup, "dup3(0) failed");
	}
	coe = fcntl(fd, F_GETFD);
	if (coe == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "fcntl failed");
	}
	if (coe & FD_CLOEXEC) {
		tst_brkm(TFAIL, cleanup,
			 "dup3(0) set close-on-exec flag");
	}
	close(fd);

	fd = syscall(__NR_dup3, 1, 4, O_CLOEXEC);
	if (fd == -1) {
		tst_brkm(TFAIL|TERRNO, cleanup, "dup3(O_CLOEXEC) failed");
	}
	coe = fcntl(fd, F_GETFD);
	if (coe == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "fcntl failed");
	}
	if ((coe & FD_CLOEXEC) == 0) {
		tst_brkm(TFAIL, cleanup,
			 "dup3(O_CLOEXEC) set close-on-exec flag");
	}
	close(fd);
	tst_resm(TPASS, "dup3(O_CLOEXEC) PASSED");

	cleanup();
	tst_exit();
}
