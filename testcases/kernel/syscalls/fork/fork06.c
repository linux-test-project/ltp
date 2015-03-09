/*
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
 *
 * NAME
 *	fork06.c
 *
 * DESCRIPTION
 *	Test that a process can fork children a large number of
 *	times in succession
 *
 * ALGORITHM
 *	Attempt to fork a child that exits immediately
 *	Repeat it many times(1000), counting the number of successes and
 *	failures
 *
 * USAGE
 *	fork06
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

char *TCID = "fork06";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

#define NUMFORKS 1000

int main(int ac, char **av)
{
	int i, pid, status, childpid, succeed, fail;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		succeed = 0;
		fail = 0;

		for (i = 0; i < NUMFORKS; i++) {
			pid = fork();
			if (pid == -1) {
				fail++;
				continue;
			}

			if (pid == 0)
				_exit(0);

			childpid = wait(&status);
			if (pid != childpid)
				tst_resm(TFAIL, "pid from wait %d", childpid);
			succeed++;
		}

		tst_resm(TINFO, "tries %d", i);
		tst_resm(TINFO, "successes %d", succeed);
		tst_resm(TINFO, "failures %d", fail);

		if ((wait(&status)) == -1)
			tst_resm(TINFO, "There were no children to wait for");
		else
			tst_resm(TINFO, "There were children left");
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
