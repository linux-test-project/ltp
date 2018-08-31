// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */
/*
 * Description:
 * Check various errnos for mlock2(2) since kernel v2.6.9:
 * 1) mlock2() fails and returns EINVAL if unknown flag is specified.
 * 2) mlock2() fails and returns ENOMEM if the caller is not
 *    privileged(CAP_IPC_LOCK) and tries to lock more memory than the
 *    RLIMIT_MEMLOCK limit.
 * 3) mlock2() fails and returns EPERM if the caller is not
 *    privileged(CAP_IPC_LOCK) and its RLIMIT_MEMLOCK limit is 0.
 * 4) mlock2() fails and returns ENOMEM if some of the specified address
 *    range does not correspond to mapped pages in the address space
 *    of the caller.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

#define PAGES 8

static size_t pgsz;
static size_t max_sz1, max_sz2;
static char *addr, *unmapped_addr;
static struct passwd *nobody;

static struct tcase {
	char **taddr;
	int flag;
	size_t *max_size;
	/* 1: nobody 0: root */
	int user;
	int exp_err;
} tcases[] = {
	{&addr, -1, NULL, 0, EINVAL},
	{&addr, 0, &max_sz1, 1, ENOMEM},
	{&addr, 0, &max_sz2, 1, EPERM},
	{&unmapped_addr, 0, NULL, 0, ENOMEM},
};

static void verify_mlock2(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct rlimit orig_limit, new_limit;

	if (tc->user) {
		SAFE_GETRLIMIT(RLIMIT_MEMLOCK, &orig_limit);
		new_limit.rlim_cur = *tc->max_size;
		new_limit.rlim_max = *tc->max_size;
		SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &new_limit);
		SAFE_SETEUID(nobody->pw_uid);
	}

	TEST(tst_syscall(__NR_mlock2, *tc->taddr, pgsz, tc->flag));
	if (TST_RET != -1) {
		tst_res(TFAIL, "mlock2() succeeded");
		SAFE_MUNLOCK(*tc->taddr, pgsz);
		goto end;
	}

	if (TST_ERR != tc->exp_err) {
		tst_res(TFAIL | TTERRNO,
			"mlock2() failed unexpectedly, expected %s",
			tst_strerrno(tc->exp_err));
	} else {
		tst_res(TPASS | TTERRNO, "mlock2() failed as expected");
	}

end:
	if (tc->user) {
		SAFE_SETEUID(0);
		SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &orig_limit);
	}
}

static void setup(void)
{
	pgsz = getpagesize();
	nobody = SAFE_GETPWNAM("nobody");

	addr = SAFE_MMAP(NULL, pgsz, PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	unmapped_addr = SAFE_MMAP(NULL, pgsz * PAGES, PROT_READ,
				  MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	SAFE_MUNMAP(unmapped_addr, pgsz * PAGES);
	unmapped_addr = unmapped_addr + pgsz * PAGES / 2;

	max_sz1 = pgsz - 1;
}

static void cleanup(void)
{
	if (addr)
		SAFE_MUNMAP(addr, pgsz);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_mlock2,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.min_kver = "2.6.9",
};
