/*
 * Copyright (c) International Business Machines  Corp., 2001
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
  HISTORY
    03/2001 - Written by Wayne Boyer

  TEST ITEMS:
    Check that a correct call to getitimer() succeeds.
*/

#include "test.h"

#include <errno.h>
#include <sys/time.h>

static void cleanup(void);
static void setup(void);

char *TCID = "getitimer01";
int TST_TOTAL = 3;

static int itimer_name[] = {
	ITIMER_REAL,
	ITIMER_VIRTUAL,
	ITIMER_PROF,
};

int main(int ac, char **av)
{
	int lc;
	int i;
	struct itimerval value;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < 3; i++) {

			TEST(getitimer(itimer_name[i], &value));

			if (TEST_RETURN != 0)
				tst_resm(TFAIL, "call failed - errno = %d - %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));

			/*
			 * Since ITIMER is effectively disabled (we did
			 * not set it before the getitimer call), the
			 * elements in it_value should be zero.
			 */
			if ((value.it_value.tv_sec == 0) &&
				(value.it_value.tv_usec == 0)) {
				tst_resm(TPASS, "functionality is ok");
			} else {
				tst_resm(TFAIL, "timer are non zero");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}
