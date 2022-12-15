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
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define EFD_NONBLOCK O_NONBLOCK

char *TCID = "eventfd2_02";
int testno;
int TST_TOTAL = 1;

void cleanup(void)
{
	tst_rmdir();
}

void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int argc, char *argv[])
{
	int fd, fl;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	tst_count = 0;
	fd = tst_syscall(__NR_eventfd2, 1, 0);
	if (fd == -1) {
		tst_brkm(TFAIL, cleanup, "eventfd2(0) failed");
	}
	fl = fcntl(fd, F_GETFL);
	if (fl == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed");
	}
	if (fl & O_NONBLOCK) {
		tst_brkm(TFAIL, cleanup, "eventfd2(0) sets non-blocking mode");
	}
	close(fd);

	fd = tst_syscall(__NR_eventfd2, 1, EFD_NONBLOCK);
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
