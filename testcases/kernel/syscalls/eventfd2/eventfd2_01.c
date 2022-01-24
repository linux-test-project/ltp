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
/* File:        eventfd2_01.c                                                 */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=b087498eb5605673b0f260a7620d91818cd72304 */
/*              says:                                                         */
/* This patch adds the new eventfd2 syscall.  It extends the old eventfd      */
/* syscall by one parameter which is meant to hold a flag value.  In this     */
/* patch the only flag support is EFD_CLOEXEC which causes the close-on-exec  */
/* flag for the returned file descriptor to be set. A new name EFD_CLOEXEC is */
/* introduced which in this implementation must have the same value as        */
/* O_CLOEXEC. The following test must be adjusted for architectures other than*/
/*  x86 and x86-64 and in case the syscall numbers changed.                   */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* eventfd2_01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                           */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   eventfd2_01                                                   */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 08 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 08 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define EFD_CLOEXEC O_CLOEXEC

char *TCID = "eventfd2_01";
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
	int fd, coe;

	tst_parse_opts(argc, argv, NULL, NULL);

	if ((tst_kvercmp(2, 6, 27)) < 0) {
		tst_brkm(TCONF, NULL,
			 "This test can only run on kernels that are 2.6.27 and higher");
	}
	setup();

	fd = tst_syscall(__NR_eventfd2, 1, 0);
	if (fd == -1) {
		tst_brkm(TFAIL, cleanup, "eventfd2(0) failed");
	}
	coe = fcntl(fd, F_GETFD);
	if (coe == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed");
	}
	if (coe & FD_CLOEXEC) {
		tst_brkm(TFAIL, cleanup, "eventfd2(0) set close-on-exec flag");
	}
	close(fd);

	fd = tst_syscall(__NR_eventfd2, 1, EFD_CLOEXEC);
	if (fd == -1) {
		tst_brkm(TFAIL, cleanup, "eventfd2(EFD_CLOEXEC) failed");
	}
	coe = fcntl(fd, F_GETFD);
	if (coe == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed");
	}
	if ((coe & FD_CLOEXEC) == 0) {
		tst_brkm(TFAIL, cleanup,
			 "eventfd2(EFD_CLOEXEC) does not set close-on-exec flag");
	}
	close(fd);
	tst_resm(TPASS, "eventfd2(EFD_CLOEXEC) Passed");
	cleanup();
	tst_exit();
}
