// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
 *
 * 06/2002 Written by Paul Larson
 */

/*\
 * Test for ENOMEM, EPERM errors.
 *
 * 1) mlock(2) fails with ENOMEM if some of the specified address range
 *    does not correspond to mapped pages in the address space of
 *    the process.
 *
 * 2) mlock(2) fails with ENOMEM if the caller had a non-zero RLIMIT_MEMLOCK
 *    soft resource limit, but tried to lock more memory than the limit
 *    permitted.  This limit is not enforced if the process is
 *    privileged (CAP_IPC_LOCK).
 *
 * 3) mlock(2) fails with EPERM if the caller was not privileged (CAP_IPC_LOCK)
 *    and its RLIMIT_MEMLOCK soft resource limit was 0.
 */

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pwd.h>
#include "tst_test.h"

static size_t len;
static struct rlimit original;
static struct passwd *ltpuser;

static void test_enomem1(void)
{
	void *addr;

	addr = SAFE_MMAP(NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	SAFE_MUNMAP(addr, len);
	TST_EXP_FAIL(mlock(addr, len), ENOMEM, "mlock(%p, %lu)", addr, len);
}

static void test_enomem2(void)
{
	void *addr;
	struct rlimit rl;

	rl.rlim_max = len - 1;
	rl.rlim_cur = len - 1;
	SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &rl);
	addr = SAFE_MMAP(NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	SAFE_SETEUID(ltpuser->pw_uid);
	TST_EXP_FAIL(mlock(addr, len), ENOMEM, "mlock(%p, %lu)", addr, len);
	SAFE_SETEUID(0);
	SAFE_MUNMAP(addr, len);
	SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &original);
}

static void test_eperm(void)
{
	void *addr;
	struct rlimit rl;

	rl.rlim_max = 0;
	rl.rlim_cur = 0;
	SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &rl);
	addr = SAFE_MMAP(NULL, len, PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	SAFE_SETEUID(ltpuser->pw_uid);
	TST_EXP_FAIL(mlock(addr, len), EPERM, "mlock(%p, %lu)", addr, len);
	SAFE_SETEUID(0);
	SAFE_MUNMAP(addr, len);
	SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &original);
}

static void run(void)
{
	test_enomem1();
	test_enomem2();
	test_eperm();
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	len = getpagesize();
	SAFE_GETRLIMIT(RLIMIT_MEMLOCK, &original);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
};
