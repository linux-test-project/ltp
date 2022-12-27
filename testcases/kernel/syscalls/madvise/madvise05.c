// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) Linux Test Project, 2014
 */
/*
 * This is a regression test for madvise(2) system call. It tests kernel
 * for NULL ptr deref Oops fixed by:
 *   commit ee53664bda169f519ce3c6a22d378f0b946c8178
 *   Author: Kirill A. Shutemov <kirill.shutemov@linux.intel.com>
 *   Date:   Fri Dec 20 15:10:03 2013 +0200
 *     mm: Fix NULL pointer dereference in madvise(MADV_WILLNEED) support
 *
 * On buggy kernel with CONFIG_TRANSPARENT_HUGEPAGE=y CONFIG_DEBUG_LOCK_ALLOC=y
 * this testcase should produce Oops and/or be killed. On fixed/good kernel
 * this testcase runs to completion (retcode is 0)
 */

#include <sys/mman.h>
#include <errno.h>
#include "tst_test.h"

#define ALLOC_SIZE (32 * 1024 * 1024)

static void verify_madvise(void)
{
	void *p;

	p = SAFE_MMAP(NULL, ALLOC_SIZE, PROT_READ,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

	TEST(mprotect(p, ALLOC_SIZE, PROT_NONE));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "mprotect failed");
	TEST(madvise(p, ALLOC_SIZE, MADV_WILLNEED));
	SAFE_MUNMAP(p, ALLOC_SIZE);

	if (TST_RET == 0) {
		tst_res(TPASS, "issue has not been reproduced");
		return;
	}

	if (TST_ERR == EBADF)
		tst_brk(TCONF, "CONFIG_SWAP=n");
	else
		tst_brk(TBROK | TTERRNO, "madvise failed");
}

static struct tst_test test = {
	.test_all = verify_madvise,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ee53664bda16"},
		{}
	}
};
