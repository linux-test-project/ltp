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
/*		rt_sigaction Expected EFAULT error check                      */
/******************************************************************************/

#define _GNU_SOURCE
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

char *TCID = "rt_sigaction02";
static int testno;
int TST_TOTAL = 1;

void cleanup(void)
{
	tst_rmdir();

	tst_exit();
}

void setup(void)
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
	EFAULT, "EFAULT"}
};

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
			for (signal = SIGRTMIN; signal <= SIGRTMAX; signal++) {
				tst_resm(TINFO, "Signal %d", signal);

				for (flag = 0; flag < ARRAY_SIZE(test_flags); flag++) {

					/*                                                              *
					 * long sys_rt_sigaction (int sig, const struct sigaction *act, *
					 * truct sigaction *oact, size_t sigsetsize);                   *
					 * EFAULT:                                                      *
					 * An invalid act or oact value was specified                   *
					 */

					TEST(ltp_rt_sigaction(signal,
						INVAL_SA_PTR, NULL, SIGSETSIZE));
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
