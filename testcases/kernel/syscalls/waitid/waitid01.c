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
/* along with this program;  if not, write to the Free Software Foundation,   */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* Description: This tests the waitid() syscall                               */
/*                                                                            */
/* Test Name:   waitid01                                                      */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "test.h"

char *TCID = "waitid01";
int testno;
int TST_TOTAL = 3;

void setup(void)
{
	TEST_PAUSE;
}

void display_status(siginfo_t * infop)
{
	tst_resm(TINFO, "Process %d terminated:", infop->si_pid);
	tst_resm(TINFO, "code = %d", infop->si_code);
	if (infop->si_code == CLD_EXITED)
		tst_resm(TINFO, "exit value = %d", infop->si_status);
	else
		tst_resm(TINFO, "signal = %d", infop->si_status);
}

int main(int ac, char **av)
{
	id_t pid;
	siginfo_t infop;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			TEST(fork());
			if (TEST_RETURN < 0)
				tst_brkm(TBROK | TTERRNO, NULL,
					"fork() failed");

			if (TEST_RETURN == 0) {
				exit(123);
			} else {
				TEST(waitid(P_ALL, getpid(), &infop, WEXITED));
				if (TEST_RETURN == -1) {
					tst_brkm(TFAIL | TTERRNO,
						 NULL,
						 "waitid(getpid()) failed");
				} else
					display_status(&infop);	//CLD_EXITED = 1
			}

			TEST(fork());
			if (TEST_RETURN < 0)
				tst_brkm(TBROK | TTERRNO, NULL,
					"fork() failed");

			if (TEST_RETURN == 0) {
				int a, b = 0;
				a = 1 / b;
				tst_exit();
			} else {
				TEST(waitid(P_ALL, 0, &infop, WEXITED));
				if (TEST_RETURN == -1) {
					tst_brkm(TFAIL | TTERRNO,
						 NULL, "waitid(0) failed");
				} else
					display_status(&infop);	//CLD_DUMPED = 3 ; SIGFPE = 8
			}

			TEST(pid = fork());
			if (TEST_RETURN < 0)
				tst_brkm(TBROK | TTERRNO, NULL,
					"fork() failed");

			if (TEST_RETURN == 0) {
				TEST(sleep(10));
				tst_exit();
			}
			TEST(kill(pid, SIGHUP));
			TEST(waitid(P_ALL, 0, &infop, WEXITED));
			if (TEST_RETURN == -1) {
				tst_brkm(TFAIL | TTERRNO, NULL,
					 "waitid(0) failed");
			} else
				display_status(&infop);	//CLD_KILLED = 2 ; SIGHUP = 1
		}
	}
	tst_resm(TPASS, "waitid(): system call passed");
	tst_exit();
}
