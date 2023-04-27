// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009 IBM Corporation.
 * Author: David Gibson
 */

/*\
 * [Description]
 *
 * Kernel has bug in mremap for some architecture. mremap() can cause
 * crashes on architectures with holes in the address space (like ia64)
 * and on powerpc with it's distict page size slices.
 *
 * This test perform mremap() with normal and hugepages around powerpc
 * slice boundary.
 */

#define _GNU_SOURCE
#include "hugetlb.h"

#define RANDOM_CONSTANT 0x1234ABCD
#define MNTPOINT "hugetlbfs/"

static int  fd = -1;
static unsigned long slice_boundary;
static unsigned long hpage_size, page_size;

static int init_slice_boundary(int fd)
{
	unsigned long slice_size;
	void *p, *heap;
	int i;
#if defined(__LP64__) && !defined(__aarch64__)
	/* powerpc: 1TB slices starting at 1 TB */
	slice_boundary = 0x10000000000;
	slice_size = 0x10000000000;
#else
	/* powerpc: 256MB slices up to 4GB */
	slice_boundary = 0x00000000;
	slice_size = 0x10000000;
#endif

	/* dummy malloc so we know where is heap */
	heap = malloc(1);
	free(heap);

	 /* Avoid underflow on systems with large huge pages.
	  * The additionally plus heap address is to reduce the possibility
	  * of MAP_FIXED stomp over existing mappings.
	  */
	while (slice_boundary + slice_size < (unsigned long)heap + 2*hpage_size)
		slice_boundary += slice_size;

	/* Find 2 neighbour slices with couple huge pages free
	 * around slice boundary.
	 * 16 is the maximum number of slices (low/high)
	 */
	for (i = 0; i < 16-1; i++) {
		slice_boundary += slice_size;
		p = mmap((void *)(slice_boundary-2*hpage_size), 4*hpage_size,
			PROT_READ, MAP_SHARED | MAP_FIXED, fd, 0);
		if (p == MAP_FAILED) {
			tst_res(TINFO|TERRNO, "can't use slice_boundary: 0x%lx",
					slice_boundary);
		} else {
			SAFE_MUNMAP(p, 4*hpage_size);
			break;
		}
	}

	if (p == MAP_FAILED) {
		tst_res(TFAIL|TERRNO, "couldn't find 2 free neighbour slices");
		return -1;
	}

	tst_res(TINFO, "using slice_boundary: 0x%lx", slice_boundary);

	return 0;
}

static void run_test(void)
{
	void *p = NULL, *q = NULL, *r;
	long p_size, q_size;
	int ret;

	fd = tst_creat_unlinked(MNTPOINT, 0);
	ret = init_slice_boundary(fd);
	if (ret)
		goto cleanup;

	/* First, hugepages above, normal below */
	tst_res(TINFO, "Testing with hpage above & normal below the slice_boundary");
	p_size = hpage_size;
	p = SAFE_MMAP((void *)(slice_boundary + hpage_size), p_size,
		 PROT_READ | PROT_WRITE,
		 MAP_SHARED | MAP_FIXED, fd, 0);

	ret = do_readback(p, p_size, "huge above");
	if (ret)
		goto cleanup;

	q_size = page_size;
	q = SAFE_MMAP((void *)(slice_boundary - page_size), q_size,
		 PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

	ret = do_readback(q, q_size, "normal below");
	if (ret)
		goto cleanup;

	r = mremap(q, page_size, 2*page_size, 0);
	if (r == MAP_FAILED) {
		tst_res(TINFO, "mremap(%p, %lu, %lu, 0) disallowed",
				q, page_size, 2*page_size);
	} else {
		q_size = 2*page_size;
		if (r != q) {
			tst_res(TFAIL, "mremap() moved without MREMAP_MAYMOVE!?");
			ret = -1;
		} else
			ret = do_readback(q, 2*page_size, "normal below expanded");
	}

	SAFE_MUNMAP(p, p_size);
	SAFE_MUNMAP(q, q_size);
	if (ret)
		goto cleanup_fd;

	/* Next, normal pages above, huge below */
	tst_res(TINFO, "Testing with normal above & hpage below the slice_boundary");
	p_size = page_size;
	p = SAFE_MMAP((void *)(slice_boundary + hpage_size), p_size,
		 PROT_READ|PROT_WRITE,
		 MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS, -1, 0);

	ret = do_readback(p, p_size, "normal above");
	if (ret)
		goto cleanup;

	q_size = hpage_size;
	q = SAFE_MMAP((void *)(slice_boundary - hpage_size),
		 q_size, PROT_READ | PROT_WRITE,
		 MAP_SHARED | MAP_FIXED, fd, 0);

	ret = do_readback(q, q_size, "huge below");
	if (ret)
		goto cleanup;

	r = mremap(q, hpage_size, 2*hpage_size, 0);
	if (r == MAP_FAILED) {
		tst_res(TINFO, "mremap(%p, %lu, %lu, 0) disallowed",
				q, hpage_size, 2*hpage_size);
	} else {
		q_size = 2*hpage_size;
		if (r != q) {
			tst_res(TFAIL, "mremap() moved without MREMAP_MAYMOVE!?");
			ret = -1;
		} else
			ret = do_readback(q, 2*hpage_size, "huge below expanded");
	}
	if (ret)
		goto cleanup;

	tst_res(TPASS, "Successful");

cleanup:
	if (p)
		SAFE_MUNMAP(p, p_size);
	if (q)
		SAFE_MUNMAP(q, q_size);
cleanup_fd:
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
	page_size = getpagesize();
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
	.hugepages = {4, TST_NEEDS},
};
