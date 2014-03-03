/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *	06/2002 Written by Paul Larson
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
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that,
 *   1. mlock() fails with -1 return value and sets errno to ENOMEM,
 *      if some of the specified address range does not correspond to
 *      mapped pages in the address space of the process.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

char *TCID = "mlock02";

#if !defined(UCLINUX)

static void setup(void);
static void cleanup(void);
static void test_enomem1(void);
static void mlock_verify(const void *, const size_t, const int);

static size_t len;

static void (*test_func[])(void) = { test_enomem1 };

int TST_TOTAL = ARRAY_SIZE(test_func);
static int exp_enos[] = { ENOMEM, 0 };

int main(int ac, char **av)
{
	int lc, i;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			(*test_func[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	len = getpagesize();
}

static void test_enomem1(void)
{
	void *addr;
	struct rlimit rl;

	/*
	 * RLIMIT_MEMLOCK resource limit.
	 * In Linux kernels before 2.6.9, this limit controlled the amount
	 * of  memory that could be locked by a privileged process. Since
	 * Linux 2.6.9, no limits are placed on the amount of memory that a
	 * privileged process may lock, and this limit instead governs the
	 * amount of memory that an unprivileged process may lock. So here
	 * we set RLIMIT_MEMLOCK resource limit to RLIM_INFINITY when kernel
	 * is under 2.6.9, to make sure this ENOMEM error is indeed caused by
	 * that some of the specified address range does not correspond to
	 * mapped pages in the address space of the process.
	 */
	if ((tst_kvercmp(2, 6, 9)) < 0) {
		rl.rlim_cur = RLIM_INFINITY;
		rl.rlim_max = RLIM_INFINITY;
		SAFE_SETRLIMIT(cleanup, RLIMIT_MEMLOCK, &rl);
	}

	addr = SAFE_MMAP(cleanup, NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	SAFE_MUNMAP(cleanup, addr, len);

	mlock_verify(addr, len, ENOMEM);
}

static void mlock_verify(const void *addr, const size_t len, const int error)
{
	TEST(mlock(addr, len));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mlock succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != error) {
		tst_resm(TFAIL | TTERRNO,
			 "mlock didn't fail as expected; expected - %d : %s",
			 error, strerror(error));
	} else {
		tst_resm(TPASS | TTERRNO, "mlock failed as expected");
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;
}

#else

int TST_TOTAL = 1;

int main(void)
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}

#endif /* if !defined(UCLINUX) */
