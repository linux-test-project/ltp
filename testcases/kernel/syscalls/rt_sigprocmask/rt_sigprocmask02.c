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
/* Description: This tests the rt_sigprocmask() syscall                       */
/*		rt_sigprocmask changes the list of currently blocked signals. */
/*		The set value stores the signal mask of the pending signals.  */
/*		The previous action on the signal is saved in oact. The value */
/*		of how indicates how the call should behave; its values are   */
/*		as follows:						      */
/*									      */
/*		SIG_BLOCK						      */
/*		    The set of blocked signals is the union of the current set*/
/*		    and the set argument. 				      */
/*		SIG_UNBLOCK						      */
/*		    The signals in set are removed from the current set of    */
/*		    blocked signals. It is okay to unblock a signal that is   */
/*		    not blocked. 					      */
/*		SIG_SETMASK						      */
/*		    The set of blocked signals is set to the set argument.    */
/*		    sigsetsize should indicate the size of a sigset_t type.   */
/* 									      */
/* 		RETURN VALUE:i						      */
/* 		rt_sigprocmask returns 0 on success; otherwise, rt_sigprocmask*/
/* 		returns one of the errors listed in the "Errors" section.     */
/* 									      */
/* 		Errors:							      */
/* 			-EINVAL						      */
/* 			    sigsetsize was not equivalent to the size of a    */
/* 			    sigset_t type or the value specified in how was   */
/* 			    invalid. 					      */
/* 			-EFAULT						      */
/* 			    An invalid set, act, or oact was specified.       */
/******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "ltp_signal.h"

char *TCID = "rt_sigprocmask02";
int TST_TOTAL = 2;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

static sigset_t set;

static struct test_case_t {
	sigset_t *ss;
	int sssize;
	int exp_errno;
} test_cases[] = {
	{
	&set, 1, EINVAL}, {
	(sigset_t *) - 1, SIGSETSIZE, EFAULT}
};

int test_count = sizeof(test_cases) / sizeof(struct test_case_t);

int main(int ac, char **av)
{
	int i;
	sigset_t s;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	tst_count = 0;

	TEST(sigfillset(&s));
	if (TEST_RETURN == -1)
		tst_brkm(TFAIL | TTERRNO, cleanup,
			"Call to sigfillset() failed.");

	for (i = 0; i < test_count; i++) {
		TEST(ltp_syscall(__NR_rt_sigprocmask, SIG_BLOCK,
			     &s, test_cases[i].ss, test_cases[i].sssize));
		if (TEST_RETURN == 0) {
			tst_resm(TFAIL | TTERRNO,
				 "Call to rt_sigprocmask() succeeded, "
				 "but should failed");
		} else if (TEST_ERRNO == test_cases[i].exp_errno) {
			tst_resm(TPASS | TTERRNO, "Got expected errno");
		} else {
			tst_resm(TFAIL | TTERRNO, "Got unexpected errno");
		}

	}

	cleanup();
	tst_exit();
}
