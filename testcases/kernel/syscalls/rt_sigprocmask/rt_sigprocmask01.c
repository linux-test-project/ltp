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
/* Inc., 59 Temple Place, Suite TEST_SIG0, Boston, MA 02111-1307 USA          */
/*                                                                            */
/* History:     Porting from Crackerjack to LTP is done by                    */
/*              Manas Kumar Nayak <maknayak@in.ibm.com>                       */
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
/*		    and the set argument.				      */
/*		SIG_UNBLOCK						      */
/*		    The signals in set are removed from the current set of    */
/*		    blocked signals. It is okay to unblock a signal that is   */
/*		    not blocked.					      */
/*		SIG_SETMASK						      */
/*		    The set of blocked signals is set to the set argument.    */
/*		    sigsetsize should indicate the size of a sigset_t type.   */
/******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "test.h"
#include "lapi/syscalls.h"
#include "lapi/rt_sigaction.h"

char *TCID = "rt_sigprocmask01";
static int testno;
int TST_TOTAL = 8;

static volatile sig_atomic_t sig_count;

#define TEST_SIG SIGRTMIN+1

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

void sig_handler(int sig)
{
	sig_count++;
}

int main(int ac, char **av)
{
	struct sigaction act, oact;
	memset(&act, 0, sizeof(act));
	memset(&oact, 0, sizeof(oact));
	act.sa_handler = sig_handler;

	sigset_t set, oset;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			if (sigemptyset(&set) < 0)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "sigemptyset call failed");

			if (sigaddset(&set, TEST_SIG) < 0)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "sigaddset call failed");

			/* call rt_sigaction() */
			TEST(ltp_rt_sigaction(TEST_SIG, &act, &oact,
						SIGSETSIZE));
			if (TEST_RETURN < 0)
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "rt_sigaction call failed");

			/* call rt_sigprocmask() to block signal#TEST_SIG */
			TEST(ltp_syscall(__NR_rt_sigprocmask, SIG_BLOCK, &set,
				     &oset, SIGSETSIZE));
			if (TEST_RETURN == -1)
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "rt_sigprocmask call failed");

			/* Make sure that the masked process is indeed
			 * masked. */
			if (kill(getpid(), TEST_SIG) < 0)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "call to kill() failed");

			if (sig_count) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "rt_sigprocmask() failed to change "
					 "the process's signal mask");
			} else {
				/* call rt_sigpending() */
				TEST(ltp_syscall(__NR_rt_sigpending, &oset,
					     SIGSETSIZE));
				if (TEST_RETURN == -1)
					tst_brkm(TFAIL | TTERRNO, cleanup,
						 "rt_sigpending call failed");

				TEST(sigismember(&oset, TEST_SIG));
				if (TEST_RETURN == 0)
					tst_brkm(TFAIL | TTERRNO,
						 cleanup,
						 "sigismember call failed");

				/* call rt_sigprocmask() to unblock
				 * signal#TEST_SIG */
				TEST(ltp_syscall(__NR_rt_sigprocmask,
					     SIG_UNBLOCK, &set, &oset,
					     SIGSETSIZE));
				if (TEST_RETURN == -1)
					tst_brkm(TFAIL | TTERRNO,
						 cleanup,
						 "rt_sigprocmask call failed");

				if (sig_count) {
					tst_resm(TPASS,
						 "rt_sigprocmask "
						 "functionality passed");
					break;
				} else {
					tst_brkm(TFAIL | TERRNO,
						 cleanup,
						 "rt_sigprocmask "
						 "functionality failed");
				}
			}

		}

		tst_count++;

	}

	cleanup();
	tst_exit();
}
