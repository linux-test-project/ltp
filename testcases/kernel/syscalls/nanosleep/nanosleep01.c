/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: nanosleep01
 *
 * Test Description:
 *  nanosleep() should return with value 0 and the process should be
 *  suspended for time specified by timespec structure.
 */

#include <errno.h>
#include "test.h"

char *TCID = "nanosleep01";
int TST_TOTAL = 1;

static void setup(void);

int main(int ac, char **av)
{
	int lc;
	struct timespec timereq = {.tv_sec = 2, .tv_nsec = 9999};

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_timer_start(CLOCK_MONOTONIC);
		TEST(nanosleep(&timereq, NULL));
		tst_timer_stop();

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TERRNO, "nanosleep() failed");
			continue;
		}

		if (tst_timespec_lt(tst_timer_elapsed(), timereq)) {
			tst_resm(TFAIL,
			         "nanosleep() suspended for %lli us, expected %lli",
				 tst_timer_elapsed_us(), tst_timespec_to_us(timereq));
		} else {
			tst_resm(TPASS, "nanosleep() suspended for %lli us",
			         tst_timer_elapsed_us());
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);
	tst_timer_check(CLOCK_MONOTONIC);
	TEST_PAUSE;
}
