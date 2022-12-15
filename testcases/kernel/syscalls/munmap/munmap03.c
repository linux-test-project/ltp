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
 *   2. munmap() fails with -1 return value and sets errno to EINVAL
 *      if the len argument is 0.
 *   3. munmap() fails with -1 return value and sets errno to EINVAL
 *      if the addr argument is not a multiple of the page size as
 *      returned by sysconf().
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
#include "safe_macros.h"

char *TCID = "munmap03";

static size_t page_sz;
static char *global_addr;
static size_t global_maplen;

static void setup(void);
static void cleanup(void);

static void test_einval1(void);
static void test_einval2(void);
static void test_einval3(void);
static void (*testfunc[])(void) = { test_einval1, test_einval2, test_einval3 };
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

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

	TEST_PAUSE;

	page_sz = (size_t)sysconf(_SC_PAGESIZE);

	global_maplen = page_sz * 2;
	global_addr = SAFE_MMAP(cleanup, NULL, global_maplen, PROT_READ |
				PROT_WRITE, MAP_PRIVATE_EXCEPT_UCLINUX |
				MAP_ANONYMOUS, -1, 0);
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

static void test_einval2(void)
{
	char *addr = global_addr;
	size_t map_len = 0;

	TEST(munmap(addr, map_len));

	check_and_print(EINVAL);
}

static void test_einval3(void)
{
	char *addr = (char *)(global_addr + 1);
	size_t map_len = page_sz;

	TEST(munmap(addr, map_len));

	check_and_print(EINVAL);
}

static void cleanup(void)
{
	if (munmap(global_addr, global_maplen) == -1)
		tst_resm(TWARN | TERRNO, "munmap failed");
}
