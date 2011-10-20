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
/* File:        rt_sigqueueinfo01.c                                           */
/*                                                                            */
/* Description: This tests the rt_sigqueueinfo() syscall.                     */
/*		rt_sigqueueinfo() Send signal information to a signal	      */
/*									      */
/* Usage:  <for command-line>                                                 */
/* rt_sigqueueinfo01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                     */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 2                                                             */
/*                                                                            */
/* Test Name:   rt_sigqueueinfo01                                              */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Global Variables */
char *TCID = "rt_sigqueueinfo01"; /* Test program identifier.*/
int testno;
int TST_TOTAL = 2; /* total number of tests in this file.   */

extern void cleanup() {

	TEST_CLEANUP;
	tst_rmdir();

}

void setup() {
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av) {
	int status;
	pid_t pid;
	pid = getpid();
	siginfo_t uinfo;

	Tst_count = 0;
	for (testno = 0; testno < TST_TOTAL; ++testno) {
		TEST(pid = fork());
		setup();
		if (TEST_RETURN < 0)
			tst_brkm(TFAIL|TTERRNO, cleanup, "fork failed");
		else if (TEST_RETURN == 0) {
			uinfo.si_errno = 0;
			uinfo.si_code = SI_QUEUE;
			TEST(syscall(__NR_rt_sigqueueinfo, getpid(), SIGCHLD,
			    &uinfo));
			if (TEST_RETURN != 0)
				err(1, "rt_sigqueueinfo");
			exit(0);
		} else {
			wait(&status);
			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				tst_resm(TPASS, "Test Succeeded");
			else
				tst_resm(TFAIL, "Test Failed");
		}
		cleanup();
	}
	tst_exit();
}
