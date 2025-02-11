// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * This test is basic shared mapping test. Two shared mappings are created
 * with same offset on a file. It checks if writing to one mapping can be
 * seen to other mapping or not?
 */

#include "hugetlb.h"

#define RANDOM_CONSTANT 0x1234ABCD
#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static int fd = -1;

static void run_test(void)
{
	void *p, *q;
	unsigned long *pl, *ql;
	unsigned long i;

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 fd, 0);

	q = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 fd, 0);

	pl = p;
	for (i = 0; i < (hpage_size / sizeof(*pl)); i++)
		pl[i] = RANDOM_CONSTANT ^ i;

	ql = q;
	for (i = 0; i < (hpage_size / sizeof(*ql)); i++) {
		if (ql[i] != (RANDOM_CONSTANT ^ i)) {
			tst_res(TFAIL, "Mismatch at offset %lu, Got: %lu, Expected: %lu",
					i, ql[i], RANDOM_CONSTANT ^ i);
			goto cleanup;
		}
	}

	tst_res(TPASS, "Successfully tested data between two shared mappings");
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
