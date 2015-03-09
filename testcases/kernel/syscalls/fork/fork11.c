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
 *
 * NAME
 *	fork11.c
 *
 * DESCRIPTION
 *	Test that parent gets a pid from each child when doing wait
 *
 * ALGORITHM
 *	Fork NUMFORKS children that do nothing.
 *
 * USAGE
 *	fork11
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
#include <errno.h>
#include "test.h"

char *TCID = "fork11";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

#define NUMFORKS 100

int main(int ac, char **av)
{
	int i, pid, cpid, status;
	int fail = 0;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < NUMFORKS; i++) {
			pid = fork();
			if (pid == 0)
				exit(0);

			if (pid > 0) {	/* parent */
				cpid = wait(&status);
				if (cpid != pid)
					fail++;
			} else {
				fail++;
				break;
			}
		}
		if (fail)
			tst_resm(TFAIL, "fork failed %d times", fail);
		else
			tst_resm(TPASS, "fork test passed, %d processes", i);
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
