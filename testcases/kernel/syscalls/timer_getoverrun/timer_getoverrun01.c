/******************************************************************************
 * Copyright (c) Crackerjack Project., 2007                                   *
 * Porting from Crackerjack to LTP is done by:                                *
 *              Manas Kumar Nayak <maknayak@in.ibm.com>                       *
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>                          *
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
#include <time.h>
#include <signal.h>
#include <sys/syscall.h>

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "timer_getoverrun01";
int TST_TOTAL = 1;

static void cleanup(void)
{

	tst_rmdir();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{
	int lc;
	int timer;
	struct sigevent ev;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	ev.sigev_value = (union sigval) 0;
	ev.sigev_signo = SIGALRM;
	ev.sigev_notify = SIGEV_SIGNAL;
	TEST(tst_syscall(__NR_timer_create, CLOCK_REALTIME, &ev, &timer));

	if (TEST_RETURN != 0)
		tst_brkm(TBROK | TTERRNO, cleanup, "Failed to create timer");

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		TEST(tst_syscall(__NR_timer_getoverrun, timer));
		if (TEST_RETURN == 0) {
			tst_resm(TPASS,
			         "timer_getoverrun(CLOCK_REALTIME) Passed");
		} else {
			tst_resm(TFAIL | TTERRNO,
			         "timer_getoverrun(CLOCK_REALTIME) Failed");
		}

		TEST(tst_syscall(__NR_timer_getoverrun, -1));
		if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL) {
			tst_resm(TPASS,	"timer_gettime(-1) Failed: EINVAL");
		} else {
			tst_resm(TFAIL | TTERRNO,
			         "timer_gettime(-1) = %li", TEST_RETURN);
		}
	}

	cleanup();
	tst_exit();
}
