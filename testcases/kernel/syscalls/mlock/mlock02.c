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
 *   2. mlock() fails with -1 return value and sets errno to ENOMEM,
 *      if (Linux  2.6.9  and  later)  the caller had a non-zero RLIMIT_MEMLOCK
 *      soft resource limit, but tried to lock more memory than the limit
 *      permitted.  This limit is not enforced if the process is privileged
 *      (CAP_IPC_LOCK).
 *   3. mlock() fails with -1 return value and sets errno to EPERM,
 *      if (Linux 2.6.9 and later) the caller was not privileged (CAP_IPC_LOCK)
 *      and its RLIMIT_MEMLOCK soft resource limit was 0.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "mlock02";

#if !defined(UCLINUX)

static void setup(void);
static void cleanup(void);
static void test_enomem1(void);
static void test_enomem2(void);
static void test_eperm(void);
static void mlock_verify(const void *, const size_t, const int);

static size_t len;
static struct rlimit original;
static struct passwd *ltpuser;

static void (*test_func[])(void) = { test_enomem1, test_enomem2, test_eperm };

int TST_TOTAL = ARRAY_SIZE(test_func);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

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
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	len = getpagesize();

	SAFE_GETRLIMIT(cleanup, RLIMIT_MEMLOCK, &original);
}

static void test_enomem1(void)
{
	void *addr;
	struct rlimit rl;

	addr = SAFE_MMAP(cleanup, NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	SAFE_MUNMAP(cleanup, addr, len);

	mlock_verify(addr, len, ENOMEM);
}

static void test_enomem2(void)
{
	void *addr;
	struct rlimit rl;

	rl.rlim_max = len - 1;
	rl.rlim_cur = len - 1;
	SAFE_SETRLIMIT(cleanup, RLIMIT_MEMLOCK, &rl);

	addr = SAFE_MMAP(cleanup, NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	mlock_verify(addr, len, ENOMEM);

	SAFE_SETEUID(cleanup, 0);

	SAFE_MUNMAP(cleanup, addr, len);

	SAFE_SETRLIMIT(cleanup, RLIMIT_MEMLOCK, &original);
}

static void test_eperm(void)
{
	void *addr;
	struct rlimit rl;

	rl.rlim_max = 0;
	rl.rlim_cur = 0;
	SAFE_SETRLIMIT(cleanup, RLIMIT_MEMLOCK, &rl);

	addr = SAFE_MMAP(cleanup, NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	mlock_verify(addr, len, EPERM);

	SAFE_SETEUID(cleanup, 0);

	SAFE_MUNMAP(cleanup, addr, len);

	SAFE_SETRLIMIT(cleanup, RLIMIT_MEMLOCK, &original);
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
}

#else

int TST_TOTAL = 1;

int main(void)
{
	tst_brkm(TCONF, NULL, "test is not available on uClinux");
}

#endif /* if !defined(UCLINUX) */
