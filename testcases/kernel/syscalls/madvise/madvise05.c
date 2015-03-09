/*
 *  Copyright (c) Linux Test Project, 2014
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
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

#include "test.h"
#include "safe_macros.h"

#define ALLOC_SIZE (32 * 1024 * 1024)

static void setup(void);
static void cleanup(void);

char *TCID = "madvise05";
int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
	int lc;
	void *p;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		p = SAFE_MMAP(cleanup, NULL, ALLOC_SIZE, PROT_READ,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
		TEST(mprotect(p, ALLOC_SIZE, PROT_NONE));
		if (TEST_RETURN == -1)
			tst_brkm(TBROK | TTERRNO, cleanup, "mprotect failed");
		TEST(madvise(p, ALLOC_SIZE, MADV_WILLNEED));
		SAFE_MUNMAP(cleanup, p, ALLOC_SIZE);

		if (TEST_RETURN == 0)
			continue;

		if (TEST_ERRNO == EBADF)
			tst_brkm(TCONF, cleanup, "CONFIG_SWAP=n");
		else
			tst_brkm(TBROK | TTERRNO, cleanup, "madvise failed");
	}

	tst_resm(TPASS, "issue has not been reproduced");

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	if (tst_kvercmp(3, 9, 0) < 0)
		tst_brkm(TCONF, NULL, "madvise(MADV_WILLNEED) swap file "
			"prefetch available only since 3.9");
	TEST_PAUSE;
}

static void cleanup(void)
{
}
