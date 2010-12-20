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
/* File:        eventfd2_02.c                                                 */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=e7d476dfdf0bcfed478a207aecfdc84f81efecaf */
/* which says:                                                                */
/* This patch adds support for the EFD_NONBLOCK flag to eventfd2.  The        */
/* additional changes needed are minimal. The following test must be adjusted */
/* or architectures other than x86 and x86-64 and in case the syscall numbers */
/* changed.                                                                   */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* eventfd2_02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                          */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   eventfd2_02                                                  */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/
#include <fcntl.h>
#include <stdio.h>
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

#define EFD_NONBLOCK O_NONBLOCK

/* Extern Global Variables */

/* Global Variables */
char *TCID = "eventfd2_02";	/* test program identifier.              */
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
	int fd, fl;

	if ((tst_kvercmp(2, 6, 27)) < 0) {
		tst_brkm(TCONF, NULL,
		    "This test can only run on kernels that are 2.6.27 and higher");
	}
	setup();

	Tst_count = 0;
	fd = syscall(__NR_eventfd2, 1, 0);
	if (fd == -1) {
		tst_brkm(TFAIL, cleanup, "eventfd2(0) failed");
	}
	fl = fcntl(fd, F_GETFL);
	if (fl == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed");
	}
	if (fl & O_NONBLOCK) {
		tst_brkm(TFAIL, cleanup,
		    "eventfd2(0) sets non-blocking mode");
	}
	close(fd);

	fd = syscall(__NR_eventfd2, 1, EFD_NONBLOCK);
	if (fd == -1) {
		tst_brkm(TFAIL, cleanup, "eventfd2(EFD_NONBLOCK) failed");
	}
	fl = fcntl(fd, F_GETFL);
	if (fl == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed");
	}
	if ((fl & O_NONBLOCK) == 0) {
		tst_brkm(TFAIL, cleanup,
			 "eventfd2(EFD_NONBLOCK) didn't set non-blocking mode");
	}
	close(fd);
	tst_resm(TPASS, "eventfd2(EFD_NONBLOCK) PASSED");

	cleanup();
	tst_exit();
}