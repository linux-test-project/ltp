// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009 IBM Corporation.
 * Author: David Gibson
 */

/*\
 * [Description]
 *
 * The kernel has bug for mremap() on some architecture. mremap() can
 * cause crashes on architectures with holes in the address space
 * (like ia64) and on powerpc with it's distinct page size "slices".
 *
 * This test get the normal mapping address and mremap() hugepage mapping
 * near to this normal mapping.
 */

#define _GNU_SOURCE
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"

static int fd = -1;
static long hpage_size;

static int do_remap(int fd, void *target)
{
	void *a, *b;
	int ret;

	a = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	ret = do_readback(a, hpage_size, "base huge");
	if (ret)
		goto cleanup;

	b = mremap(a, hpage_size, hpage_size, MREMAP_MAYMOVE | MREMAP_FIXED,
		   target);

	if (b != MAP_FAILED) {
		ret = do_readback(b, hpage_size, "remapped");
		a = b;
	} else {
		tst_res(TINFO|TERRNO, "mremap(MAYMOVE|FIXED) disallowed");
	}

cleanup:
	SAFE_MUNMAP(a, hpage_size);
	return ret;
}

static void *map_align(size_t size, size_t align)
{
	unsigned long xsize = size + align - getpagesize();
	size_t t;
	void *p, *q;

	p = SAFE_MMAP(NULL, xsize, PROT_READ|PROT_WRITE,
		 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	q = PALIGN(p, align);

	t = q - p;
	if (t)
		SAFE_MUNMAP(p, t);

	t = p + xsize - (q + size);
	if (t)
		SAFE_MUNMAP(q + size, t);

	return q;
}

static void run_test(void)
{
	void *p;
	int ret;

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	p = map_align(3*hpage_size, hpage_size);

	SAFE_MUNMAP(p, hpage_size);
	SAFE_MUNMAP(p + 2*hpage_size, hpage_size);

	p = p + hpage_size;

	tst_res(TINFO, "Normal mapping at %p", p);
	ret = do_readback(p, hpage_size, "base normal page");
	if (ret)
		goto cleanup;

	ret = do_remap(fd, p - hpage_size);
	if (ret)
		goto cleanup;

	ret = do_remap(fd, p + hpage_size);
	if (ret == 0)
		tst_res(TPASS, "Successfully tested mremap hpage near normal mapping");

cleanup:
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {3, TST_NEEDS},
};
