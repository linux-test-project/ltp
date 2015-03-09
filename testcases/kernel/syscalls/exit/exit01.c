/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Check that exit returns the correct values to the waiting parent
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "test.h"

static void cleanup(void);
static void setup(void);

char *TCID = "exit01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int pid, npid, sig, nsig, exno, nexno, status;
	int rval = 0;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		sig = 0;
		exno = 1;

		pid = FORK_OR_VFORK();

		switch (pid) {
		case 0:
			exit(exno);
		break;
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");
		break;
		default:
			npid = wait(&status);

			if (npid != pid) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected pid returned");
				rval = 1;
			}

			nsig = status % 256;

			/*
			 * Check if the core dump bit has been set, bit # 7
			 */
			if (nsig >= 128) {
				nsig = nsig - 128;
			}

			/*
			 * nsig is the signal number returned by wait
			 */
			if (nsig != sig) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected signal returned");
				rval = 1;
			}

			/*
			 * nexno is the exit number returned by wait
			 */
			nexno = status / 256;
			if (nexno != exno) {
				tst_resm(TFAIL, "wait error: "
					 "unexpected exit number returned");
				rval = 1;
			}
		break;
		}

		if (rval != 1) {
			tst_resm(TPASS, "exit() test PASSED");
		}
	}
	
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
