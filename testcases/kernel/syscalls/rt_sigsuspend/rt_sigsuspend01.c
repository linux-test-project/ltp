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
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak maknayak@in.ibm.com>                        */
/******************************************************************************/

/******************************************************************************/
/* Description: This tests the rt_sigsuspend() syscall.                       */
/******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "lapi/rt_sigaction.h"

char *TCID = "rt_sigsuspend01";
int TST_TOTAL = 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

static void sig_handler(int sig)
{
}

int main(int ac, char **av)
{
	sigset_t set, set1, set2;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {

		tst_count = 0;

		if (sigemptyset(&set) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "sigemptyset failed");
		struct sigaction act, oact;
		memset(&act, 0, sizeof(act));
		memset(&oact, 0, sizeof(oact));
		act.sa_handler = sig_handler;

		TEST(ltp_rt_sigaction(SIGALRM, &act, &oact, SIGSETSIZE));
		if (TEST_RETURN == -1)
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "rt_sigaction failed");

		TEST(ltp_syscall(__NR_rt_sigprocmask, SIG_UNBLOCK, 0,
			     &set1, SIGSETSIZE));
		if (TEST_RETURN == -1)
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "rt_sigprocmask failed");

		TEST(alarm(5));
		int result;
		TEST(result = ltp_syscall(__NR_rt_sigsuspend, &set,
			SIGSETSIZE));
		TEST(alarm(0));
		if (result == -1 && TEST_ERRNO != EINTR) {
			TEST(ltp_syscall(__NR_rt_sigprocmask, SIG_UNBLOCK, 0,
				&set2, SIGSETSIZE));
			if (TEST_RETURN == -1) {
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "rt_sigprocmask failed");
			} else if (set1.__val[0] != set2.__val[0]) {
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "rt_sigsuspend failed to "
					 "preserve signal mask");
			} else {
				tst_resm(TPASS, "rt_sigsuspend PASSED");
			}
		} else {
			tst_resm(TFAIL | TTERRNO, "rt_sigsuspend failed");
		}

	}

	cleanup();

	tst_exit();
}
