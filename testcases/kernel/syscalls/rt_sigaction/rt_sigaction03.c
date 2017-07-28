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
/* Description: This tests the rt_sigaction() syscall                         */
/*		rt_sigaction Expected EINVAL error check                      */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "lapi/rt_sigaction.h"

#define INVAL_SIGSETSIZE -1

char *TCID = "rt_sigaction03";
static int testno;
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

static int test_flags[] =
    { SA_RESETHAND | SA_SIGINFO, SA_RESETHAND, SA_RESETHAND | SA_SIGINFO,
SA_RESETHAND | SA_SIGINFO, SA_NOMASK };
char *test_flags_list[] =
    { "SA_RESETHAND|SA_SIGINFO", "SA_RESETHAND", "SA_RESETHAND|SA_SIGINFO",
"SA_RESETHAND|SA_SIGINFO", "SA_NOMASK" };

static struct test_case_t {
	int exp_errno;
	char *errdesc;
} test_cases[] = {
	{
	EINVAL, "EINVAL"}
};

static void handler(int sig)
{
	tst_resm(TINFO, "Signal Handler Called with signal number %d\n", sig);
	return;
}

static int set_handler(int sig, int sig_to_mask, int mask_flags)
{
	struct sigaction sa, oldaction;

	sa.sa_sigaction = (void *)handler;
	sa.sa_flags = mask_flags;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, sig_to_mask);

	/*                                                              *
	 * long sys_rt_sigaction (int sig, const struct sigaction *act, *
	 * truct sigaction *oact, size_t sigsetsize);                   *
	 * EINVAL:                                                      *
	 * sigsetsize was not equivalent to the size of a sigset_t type *
	 */

	return ltp_rt_sigaction(sig, &sa, &oldaction, INVAL_SIGSETSIZE);
}

int main(int ac, char **av)
{
	unsigned int flag;
	int signal;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			for (signal = SIGRTMIN; signal <= (SIGRTMAX); signal++) {
				tst_resm(TINFO, "Signal %d", signal);

				for (flag = 0; flag < ARRAY_SIZE(test_flags); flag++) {
					TEST(set_handler
					     (signal, 0, test_flags[flag]));
					if ((TEST_RETURN == -1)
					    && (TEST_ERRNO ==
						test_cases[0].exp_errno)) {
						tst_resm(TINFO,
							 "sa.sa_flags = %s ",
							 test_flags_list[flag]);
						tst_resm(TPASS,
							 "%s failure with sig: %d as expected errno  = %s : %s",
							 TCID, signal,
							 test_cases[0].errdesc,
							 strerror(TEST_ERRNO));
					} else {
						tst_resm(TFAIL,
							 "rt_sigaction call succeeded: result = %ld got error %d:but expected  %d",
							 TEST_RETURN,
							 TEST_ERRNO,
							 test_cases[0].
							 exp_errno);
						tst_resm(TINFO,
							 "sa.sa_flags = %s ",
							 test_flags_list[flag]);
					}
				}
			}

		}
	}
	cleanup();
	tst_exit();
}
