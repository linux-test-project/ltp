/*
 *
 *   Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
 *   Author: Suresh Babu V. <suresh.babu@wipro.com>
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
 * Verify that getrlimit(2) call will be successful for all possible resource
 * types.
 */
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "test.h"

static void cleanup(void);
static void setup(void);

static struct rlimit rlim;
static struct test_t {
	int res;
	char *res_str;
} testcases[] = {
	{RLIMIT_CPU, "RLIMIT_CPU"},
	{RLIMIT_FSIZE, "RLIMIT_FSIZE"},
	{RLIMIT_DATA, "RLIMIT_DATA"},
	{RLIMIT_STACK, "RLIMIT_STACK"},
	{RLIMIT_CORE, "RLIMIT_CORE"},
	{RLIMIT_RSS, "RLIMIT_RSS"},
	{RLIMIT_NPROC, "RLIMIT_NPROC"},
	{RLIMIT_NOFILE, "RLIMIT_NOFILE"},
	{RLIMIT_MEMLOCK, "RLIMIT_MEMLOCK"},
	{RLIMIT_AS, "RLIMIT_AS"},
	{RLIMIT_LOCKS, "RLIMIT_LOCKS"},
	{RLIMIT_MSGQUEUE, "RLIMIT_MSGQUEUE"},
#ifdef RLIMIT_NICE
	{RLIMIT_NICE, "RLIMIT_NICE"},
#endif
#ifdef RLIMIT_RTPRIO
	{RLIMIT_RTPRIO, "RLIMIT_RTPRIO"},
#endif
	{RLIMIT_SIGPENDING, "RLIMIT_SIGPENDING"},
#ifdef RLIMIT_RTTIME
	{RLIMIT_RTTIME, "RLIMIT_RTTIME"},
#endif
};

char *TCID = "getrlimit01";
int TST_TOTAL = ARRAY_SIZE(testcases);

int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			TEST(getrlimit(testcases[i].res, &rlim));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO,
					 "getrlimit() test %s failed",
					 testcases[i].res_str);
			} else {
				tst_resm(TPASS,
					 "getrlimit() test %s success",
					 testcases[i].res_str);
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
