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
 * Test Description:
 *  Verify that,
 *   1. munmap() fails with -1 return value and sets errno to EINVAL
 *      if addresses in the range [addr,addr+len) are outside the valid
 *	range for the address space of a process.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

char *TCID = "munmap03";

static size_t page_sz;
static int exp_enos[] = { EINVAL, 0 };

static void setup(void);
static void cleanup(void);

static void test_einval1(void);
static void (*testfunc[])(void) = { test_einval1 };
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int i, lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	page_sz = (size_t)sysconf(_SC_PAGESIZE);
}

static void check_and_print(int expected_errno)
{
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == expected_errno) {
			tst_resm(TPASS | TTERRNO, "failed as expected");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "failed unexpectedly; expected - %d : %s",
				 expected_errno, strerror(expected_errno));
		}
	} else {
		tst_resm(TFAIL, "munmap succeeded unexpectedly");
	}
}

static void test_einval1(void)
{
	struct rlimit brkval;
	char *addr;
	size_t map_len;

	SAFE_GETRLIMIT(cleanup, RLIMIT_DATA, &brkval);

	addr = (char *)brkval.rlim_max;
	map_len = page_sz * 2;

	TEST(munmap(addr, map_len));

	check_and_print(EINVAL);
}

static void cleanup(void)
{
	TEST_CLEANUP;

}
