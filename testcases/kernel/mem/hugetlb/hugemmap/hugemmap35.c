// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that :manpage:`mmap(2)` accepts ``PROT_MTE`` for hugetlbfs file
 * mappings that do not specify ``MAP_HUGETLB``.
 *
 * On arm64, hugetlbfs file mappings must get ``VM_MTE_ALLOWED`` before mmap
 * flag validation. This is needed because mapping a hugetlbfs file does not
 * have to use ``MAP_HUGETLB``. The test needs root to reserve huge pages and
 * mount hugetlbfs.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "hugetlb.h"
#include "lapi/mman.h"
#include "lapi/mmap.h"

#define MNTPOINT "hugetlbfs/"

static int fd = -1;
static long hpage_size;

#define TC(x) {x, #x}

static struct tcase {
	int flags;
	const char *desc;
} tcases[] = {
	TC(MAP_SHARED),
	TC(MAP_PRIVATE),
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	volatile char *ptr;

	TESTPTR(mmap(NULL, hpage_size, PROT_READ | PROT_WRITE | PROT_MTE,
		     tc->flags, fd, 0));

	if (TST_RET_PTR == MAP_FAILED) {
		if (TST_ERR == ENOMEM)
			tst_brk(TCONF, "Not enough huge pages");

		tst_res(TFAIL | TTERRNO, "mmap(PROT_MTE, %s) without MAP_HUGETLB failed",
			tc->desc);
		return;
	}

	ptr = TST_RET_PTR;
	*ptr = 1;

	SAFE_MUNMAP(TST_RET_PTR, hpage_size);

	tst_res(TPASS, "mmap(PROT_MTE, %s) without MAP_HUGETLB passed", tc->desc);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
	if (!hpage_size)
		tst_brk(TCONF, "Huge pages are not supported");

	TESTPTR(mmap(NULL, hpage_size, PROT_READ | PROT_MTE,
		     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0));

	if (TST_RET_PTR == MAP_FAILED)
		tst_brk(TCONF | TTERRNO, "PROT_MTE is not supported with MAP_HUGETLB");

	SAFE_MUNMAP(TST_RET_PTR, hpage_size);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	SAFE_FTRUNCATE(fd, hpage_size);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.hugepages = { 2, TST_NEEDS },
	.supported_archs = (const char *const []) {
		"aarch64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "49ccf2c3cafb"},
		{}
	},
};
