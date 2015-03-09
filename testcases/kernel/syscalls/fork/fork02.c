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
 *	fork02.c
 *
 * DESCRIPTION
 *	Test correct operation of fork:
 *		pid == 0 in child;
 *		pid > 0 in parent from wait;
 *
 * ALGORITHM
 *	Fork one process, check for pid == 0 in child.
 *	Check for pid > 0 in parent after wait.
 *
 * USAGE
 *	fork02
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
#include <unistd.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

char *TCID = "fork02";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
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
			tst_resm(TINFO, "Inside child");
			_exit(0);
		} else {
			tst_resm(TINFO, "Inside parent");
			pid2 = wait(&status);
			tst_resm(TINFO, "exit status of wait %d", status);

			if (pid1 == pid2)
				tst_resm(TPASS, "test 1 PASSED");
			else
				tst_resm(TFAIL, "test 1 FAILED");
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
