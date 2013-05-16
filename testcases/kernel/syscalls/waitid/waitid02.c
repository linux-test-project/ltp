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
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation,  Inc.,   */
/* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA                 */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        waitid02.c                                           	      */
/*                                                                            */
/* Description: This tests the waitid() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* waitid02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                              */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   waitid02                                                      */
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
#include "usctest.h"
#include "linux_syscall_numbers.h"

char *TCID = "waitid02";
int testno;
int TST_TOTAL = 4;

static void cleanup(void)
{
	TEST_CLEANUP;
	tst_rmdir();

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{
	id_t pgid;
	id_t id1, id2, id3;
	siginfo_t infop;

	int lc;
	char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			TEST(waitid(P_ALL, 0, &infop, WNOHANG));
			if (TEST_RETURN == -1)
				tst_resm(TPASS, "Success1 ... -1 is returned."
					" error is %d.", TEST_ERRNO);
			else {
				tst_resm(TFAIL, "%s Failed1 ...", TCID);
			}

			/* option == WEXITED | WCONTINUED | WSTOPPED |
			 * WNOHANG | WNOWAIT */

			TEST(id1 = fork());
			if (TEST_RETURN == 0) {
				tst_resm(TINFO,
					 "I'm a child 1,my id is %d,gpid is %d",
					 id1 = getpid(), __getpgid(0));
				sleep(1);
				exit(5);
			}

			TEST(id2 = fork());
			if (TEST_RETURN == 0) {
				sleep(3);
				tst_resm(TINFO,
					 "I'm a child 2,my id is %d,gpid is %d",
					 id2 = getpid(), __getpgid(0));
				exit(7);
			}

			TEST(id3 = fork());
			if (TEST_RETURN == 0) {
				sleep(2);
				TEST(kill(id2, SIGCONT));
				tst_resm(TINFO,
					 "I'm a child 3,my id is %d,gpid is %d",
					 id3 = getpid(), __getpgid(0));
				exit(6);
			}

			TEST(waitid(P_ALL, 0, &infop, WNOHANG | WEXITED));
			if (TEST_RETURN == 0)
				tst_resm(TPASS, "Success 2 ...0 is returned.."
					" error is %d.", TEST_ERRNO);
			else {
				tst_resm(TFAIL | TTERRNO, "%s Failed 2", TCID);
				tst_exit();
			}

			tst_resm(TINFO, "I'm a Parent,my id is %d,gpid is %d",
				 getpid(), pgid = __getpgid(0));

			TEST(waitid(P_PGID, pgid, &infop, WEXITED));
			if (TEST_RETURN == 0) {
				tst_resm(TPASS, "Success3 ... 0 is returned.");
				tst_resm(TINFO, "si_pid = %d ; si_code = %d ;"
					" si_status = %d",
					 infop.si_pid, infop.si_code,
					 infop.si_status);
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "Fail3 ...  %ld is returned",
					 TEST_RETURN);
				tst_exit();
			}

			TEST(kill(id2, SIGSTOP));

			TEST(waitid(P_PID, id2, &infop, WSTOPPED | WNOWAIT));
			if (TEST_RETURN == 0) {
				/*EINVAL*/
				tst_resm(TINFO,	"si_pid = %d, si_code = %d,"
					" si_status = %d",
					infop.si_pid, infop.si_code,
					infop.si_status);
				tst_resm(TPASS, "Success4 ... 0 is returned");
			} else {
				tst_resm(TFAIL | TTERRNO,
					"Fail4 ...  %ld is returned",
					TEST_RETURN);
				tst_exit();
			}

			TEST(waitid(P_PID, id3, &infop, WEXITED));
			if (TEST_RETURN == 0) {
				/*NOCHILD*/
				tst_resm(TINFO,
					"si_pid = %d, si_code = %d, "
					"si_status = %d",
					infop.si_pid, infop.si_code,
					infop.si_status);
				tst_resm(TPASS, "Success5 ... 0 is returned");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "Fail5 ...  %ld is returned",
					 TEST_RETURN);
				tst_exit();
			}

			TEST(waitid(P_PID, id2, &infop, WCONTINUED));
			if (TEST_RETURN == 0) {
				/*EINVAL*/
				tst_resm(TINFO,
					"si_pid = %d, si_code = %d, "
					"si_status = %d",
					infop.si_pid, infop.si_code,
					infop.si_status);
				tst_resm(TPASS, "Success6 ... 0 is returned");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "Fail6 ...  %ld is returned",
					 TEST_RETURN);
				tst_exit();
			}

			sleep(3);
		}
	}
	cleanup();
	tst_exit();
}
