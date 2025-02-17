// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (C) 2006 Hugh Dickins <hugh@veritas.com>
 * Author: David Gibson & Adam Litke
 */

/*\
 * At one stage, a misconversion of hugetlb_vmtruncate_list to a
 * prio_tree meant that on 32-bit machines, truncates at or above 4GB
 * could truncate lower pages, resulting in BUG_ON()s.
 *
 * WARNING: The offsets and addresses used within are specifically
 * calculated to trigger the bug as it existed.  Don't mess with them
 * unless you *really* know what you're doing.
 *
 * The kernel bug in question was fixed with commit
 * 856fc2950555.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"

#define RANDOM_CONSTANT	0x1234ABCD
#define MNTPOINT "hugetlbfs/"
static int page_size;
static long hpage_size;
static int  fd = -1;

static void run_test(void)
{
	off_t buggy_offset;
	void *p, *q;
	volatile int *pi;
	int err;

	/*
	 * First, we make a 2 page sane hugepage mapping.  Then we
	 * memset() it to ensure that the ptes are instantiated for
	 * it.  Then we attempt to replace the second half of the map
	 * with one at a bogus offset.  We leave the first page of
	 * sane mapping in place to ensure that the corresponding
	 * pud/pmd/whatever entries aren't cleaned away.  It's those
	 * bad entries which can trigger bad_pud() checks if the
	 * backout path for the bogus mapping is buggy, which it was
	 * in some kernels.
	 */
	tst_res(TINFO, "Initial free hugepages: %lu",
		SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE));

	/* First get arena of three hpages size, at file offset 4GB */
	p = SAFE_MMAP(NULL, 2*hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

	tst_res(TINFO, "After Mapping reference map, Free hugepages: %lu",
		SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE));
	tst_res(TINFO, "Mapped Address Range: %p-%p", p, p+2*hpage_size-1);

	memset(p, 0, 2*hpage_size);
	pi = p;
	*pi = RANDOM_CONSTANT;

	tst_res(TINFO, "After instantiate the pages, Free hugepages: %lu",
		   SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE));

	/*
	 * Toggle the permissions on the first page.  This forces TLB
	 * entries (including hash page table on powerpc) to be
	 * flushed, so that the page tables must be accessed for the
	 * test further down.  In the buggy case, those page tables
	 * can get thrown away by a pud_clear()
	 */
	err = mprotect(p, hpage_size, PROT_READ);
	if (err)
		tst_brk(TBROK|TERRNO, "mprotect(%p, 0x%lx, PROT_READ)", p, hpage_size);

	/* Replace top hpage by hpage mapping at confusing file offset */
	buggy_offset = page_size;
	tst_res(TINFO, "Replacing map at %p with map from offset 0x%lx...",
	       p + hpage_size, (unsigned long)buggy_offset);
	q = mmap(p + hpage_size, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_FIXED|MAP_PRIVATE, fd, buggy_offset);
	if (q != MAP_FAILED) {
		tst_res(TFAIL|TERRNO, "bogus offset mmap() succeeded at %p", q);
		goto cleanup;
	}
	if (errno != EINVAL) {
		tst_res(TFAIL|TERRNO, "bogus mmap() failed should be \"%s\" but it is",
		     tst_strerrno(EINVAL));
		goto cleanup;
	}

	tst_res(TINFO, "After Mapping with buggy offset, Free hugepages: %lu",
		SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE));

	if (*pi != RANDOM_CONSTANT) {
		tst_res(TFAIL, "Pre-existing mapping clobbered: %x instead of %x",
		     *pi, RANDOM_CONSTANT);
		goto cleanup;
	}

	/*
	 * The real test is whether we got a bad_pud() or similar
	 * during the run.  The check above, combined with the earlier
	 * mprotect()s to flush the TLB are supposed to catch it, but
	 * it's hard to be certain.  Once bad_pud() is called
	 * behaviour can be very strange.
	 */

	tst_res(TPASS, "Successful but inconclusive");
cleanup:
	SAFE_MUNMAP(p, 2*hpage_size);
}

static void setup(void)
{
	page_size = getpagesize();
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "856fc2950555"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {4, TST_NEEDS},
};
