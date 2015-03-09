/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	fork03.c
 *
 * DESCRIPTION
 *	Check that child can use a large text space and do a large
 *	number of operations.
 *
 * ALGORITHM
 *	Fork one process, check for pid == 0 in child.
 *	Check for pid > 0 in parent after wait.
 *
 * USAGE
 *	fork03
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "test.h"

char *TCID = "fork03";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	float fl1, fl2;
	int i;
	int pid1, pid2, status;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		pid1 = fork();
		if (pid1 == -1)
			tst_brkm(TBROK, cleanup, "fork() failed");

		if (pid1 == 0) {
			/* child uses some cpu cycles */
			for (i = 1; i < 32767; i++) {
				fl1 = 0.000001;
				fl1 = fl2 = 0.000001;
				fl1 = fl1 * 10.0;
				fl2 = fl1 / 1.232323;
				fl1 = fl2 - fl2;
				fl1 = fl2;
			}

			/* Pid must always be zero in child  */
			if (pid1 != 0)
				exit(1);
			else
				exit(0);
		} else {
			tst_resm(TINFO, "process id in parent of child from "
				 "fork : %d", pid1);
			pid2 = wait(&status);	/* wait for child */

			if (pid1 != pid2) {
				tst_resm(TFAIL, "pids don't match : %d vs %d",
					 pid1, pid2);
				continue;
			}

			if ((status >> 8) != 0) {
				tst_resm(TFAIL, "child exited with failure");
				continue;
			}

			tst_resm(TPASS, "test 1 PASSED");
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
