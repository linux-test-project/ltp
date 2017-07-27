/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007                                   *
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
/*
 * File:	tkill02.c
 *
 * Description: This tests the tkill() syscall
 *
 * History:     Porting from Crackerjack to LTP is done by
 *              Manas Kumar Nayak maknayak@in.ibm.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "tkill02";
int testno;

static pid_t inval_tid = -1;
static pid_t unused_tid;

void cleanup(void)
{
	tst_rmdir();
}

void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();

	unused_tid = tst_get_unused_pid(cleanup);
}

struct test_case_t {
	int *tid;
	int exp_errno;
} test_cases[] = {
	{&inval_tid, EINVAL},
	{&unused_tid, ESRCH}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{
	int i;

	setup();

	tst_parse_opts(ac, av, NULL, NULL);

	for (i = 0; i < TST_TOTAL; i++) {

		TEST(ltp_syscall(__NR_tkill, *(test_cases[i].tid), SIGUSR1));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == test_cases[i].exp_errno) {
				tst_resm(TPASS | TTERRNO,
					 "tkill(%d, SIGUSR1) failed as expected",
					 *(test_cases[i].tid));
			} else {
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "tkill(%d, SIGUSR1) failed unexpectedly",
					 *(test_cases[i].tid));
			}
		} else {
			tst_brkm(TFAIL, cleanup,
				 "tkill(%d) succeeded unexpectedly",
				 *(test_cases[i].tid));
		}
	}
	cleanup();
	tst_exit();
}
