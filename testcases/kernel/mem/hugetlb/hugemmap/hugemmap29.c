// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * The test do mmap() with shared mapping and write. It matches the data
 * with private mmap() and then change it with other data. It checks
 * shared mapping data if data is not contaiminated by private mapping.
 * Similiarly checks for private data if it is not contaminated by changing
 * shared mmap data.
 */

#include "hugetlb.h"

#define C1 0x1234ABCD
#define C2 0xFEDC9876

#define MNTPOINT "hugetlbfs/"
static unsigned long hpage_size;
static int fd = -1;

static void run_test(void)
{
	void *p, *q;
	unsigned int *pl, *ql;
	unsigned long i;

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 fd, 0);

	pl = p;
	for (i = 0; i < (hpage_size / sizeof(*pl)); i++)
		pl[i] = C1 ^ i;

	q = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE,
		 fd, 0);

	ql = q;
	for (i = 0; i < (hpage_size / sizeof(*ql)); i++) {
		if (ql[i] != (C1 ^ i)) {
			tst_res(TFAIL, "Mismatch at offset %lu, got: %u, expected: %lu",
					i, ql[i], C1 ^ i);
			goto cleanup;
		}
	}

	for (i = 0; i < (hpage_size / sizeof(*ql)); i++)
		ql[i] = C2 ^ i;

	for (i = 0; i < (hpage_size / sizeof(*ql)); i++) {
		if (ql[i] != (C2 ^ i)) {
			tst_res(TFAIL, "PRIVATE mismatch at offset %lu, got: %u, expected: %lu",
					i, ql[i], C2 ^ i);
			goto cleanup;
		}
	}

	for (i = 0; i < (hpage_size / sizeof(*pl)); i++) {
		if (pl[i] != (C1 ^ i)) {
			tst_res(TFAIL, "SHARED map contaminated at offset %lu, "
					"got: %u, expected: %lu", i, pl[i], C1 ^ i);
			goto cleanup;
		}
	}

	memset(p, 0, hpage_size);

	for (i = 0; i < (hpage_size / sizeof(*ql)); i++) {
		if (ql[i] != (C2 ^ i)) {
			tst_res(TFAIL, "PRIVATE map contaminated at offset %lu, "
					"got: %u, expected: %lu", i, ql[i], C2 ^ i);
			goto cleanup;
		}
	}
	tst_res(TPASS, "Successfully tested shared/private mmaping and its data");
cleanup:
	SAFE_MUNMAP(p, hpage_size);
	SAFE_MUNMAP(q, hpage_size);
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
	.hugepages = {2, TST_NEEDS},
};
