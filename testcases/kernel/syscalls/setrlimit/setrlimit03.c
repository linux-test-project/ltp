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
 *	setrlimit03.c
 *
 * DESCRIPTION
 *	Test for EPERM when the super-user tries to increase RLIMIT_NOFILE
 *	beyond the system limit.
 *
 * USAGE:  <for command-line>
 *  setrlimit03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Must run test as root.
 */
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include <linux/fs.h>

char *TCID = "setrlimit03";
int TST_TOTAL = 1;

#if !defined(NR_OPEN)
//Taken from definition in /usr/include/linux/fs.h
#define NR_OPEN (1024*1024)
#endif

void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;
	struct rlimit rlim;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		if (getrlimit(RLIMIT_NOFILE, &rlim) != 0)
			tst_brkm(TFAIL, cleanup, "getrlimit failed, "
				 "errno = %d", errno);
		rlim.rlim_max = NR_OPEN + 1;

		TEST(setrlimit(RLIMIT_NOFILE, &rlim));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		if (TEST_ERRNO != EPERM) {
			tst_resm(TFAIL, "Expected EPERM, got %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "got expected EPERM error");
		}
	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}
