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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
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

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "rt_sigqueueinfo01";
int testno;
int TST_TOTAL = 2;

void cleanup(void)
{

	tst_rmdir();

}

void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int main(void)
{
	int status;
	pid_t pid;
	pid = getpid();
	siginfo_t uinfo;

	tst_count = 0;
	for (testno = 0; testno < TST_TOTAL; ++testno) {
		TEST(pid = fork());
		setup();
		if (TEST_RETURN < 0)
			tst_brkm(TFAIL | TTERRNO, cleanup, "fork failed");
		else if (TEST_RETURN == 0) {
			uinfo.si_errno = 0;
			uinfo.si_code = SI_QUEUE;
			TEST(ltp_syscall(__NR_rt_sigqueueinfo, getpid(),
				SIGCHLD, &uinfo));
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
