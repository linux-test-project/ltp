/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007                                   *
 * Ported to LTP by Manas Kumar Nayak <maknayak@in.ibm.com>                   *
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>                          *
 *                                                                            *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  *
 * the GNU General Public License for more details.                           *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *                                                                            *
 ******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <linux/unistd.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "exit_group01";
int testno;
int TST_TOTAL = 1;

static void verify_exit_group(void)
{
	pid_t cpid, w;
	int status;

	cpid = fork();
	if (cpid == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "fork failed");

	if (cpid == 0) {
		TEST(tst_syscall(__NR_exit_group, 4));
	} else {
		w = SAFE_WAIT(NULL, &status);

		if (WIFEXITED(status) && (WEXITSTATUS(status) == 4)) {
			tst_resm(TPASS, "exit_group() succeeded");
		} else {
			tst_resm(TFAIL | TERRNO,
				 "exit_group() failed (wait status = %d)", w);
		}
	}
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_exit_group();

	tst_exit();
}
