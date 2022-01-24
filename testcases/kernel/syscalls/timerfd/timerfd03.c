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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        timerfd03.c                                                   */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=6b1ef0e60d42f2fdaec26baee8327eb156347b4f */
/* which says:                                                                */
/* This patch adds support for the TFD_NONBLOCK flag to timerfd_create.  The  */
/* additional changes needed are minimal. The following test must be adjusted */
/* for architectures other than x86 and x86-64 and in case the syscall numbers*/
/* changed.                                                                   */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* timerfd03 [-c n] [-e][-i n] [-I x] [-p x] [-t]                          */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   timerfd03                                                  */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define TFD_NONBLOCK O_NONBLOCK

char *TCID = "timerfd03";
int testno;
int TST_TOTAL = 1;

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
void cleanup(void)
{

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
void setup(void)
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int argc, char *argv[])
{
	int fd, fl;
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);
	if ((tst_kvercmp(2, 6, 27)) < 0) {
		tst_brkm(TCONF,
			 NULL,
			 "This test can only run on kernels that are 2.6.27 and higher");
	}
	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {
			fd = tst_syscall(__NR_timerfd_create,
				CLOCK_REALTIME, 0);
			if (fd == -1) {
				tst_brkm(TFAIL, cleanup,
					 "timerfd_create(0) failed");
			}
			fl = fcntl(fd, F_GETFL);
			if (fl == -1) {
				tst_brkm(TBROK, cleanup, "fcntl failed");
			}
			if (fl & O_NONBLOCK) {
				tst_brkm(TFAIL,
					 cleanup,
					 "timerfd_create(0) set non-blocking mode");
			}
			close(fd);

			fd = tst_syscall(__NR_timerfd_create, CLOCK_REALTIME,
				     TFD_NONBLOCK);
			if (fd == -1) {
				tst_brkm(TFAIL,
					 cleanup,
					 "timerfd_create(TFD_NONBLOCK) failed");
			}
			fl = fcntl(fd, F_GETFL);
			if (fl == -1) {
				tst_brkm(TBROK, cleanup, "fcntl failed");
			}
			if ((fl & O_NONBLOCK) == 0) {
				tst_brkm(TFAIL,
					 cleanup,
					 "timerfd_create(TFD_NONBLOCK) set non-blocking mode");
			}
			close(fd);
			tst_resm(TPASS, "timerfd_create(TFD_NONBLOCK) PASSED");
			cleanup();
		}
	}
	tst_exit();
}
