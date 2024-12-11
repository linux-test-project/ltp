// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * The test checks that mlocking hugetlb areas works with all combinations
 * of MAP_PRIVATE and MAP_SHARED with and without MAP_LOCKED specified.
 */

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
#define FLAGS_DESC(x) .flags = x, .flags_str = #x

static int fd = -1;
static unsigned long hpage_size;

static struct tcase {
	int flags;
	char *flags_str;
} tcases[] = {
	{ FLAGS_DESC(MAP_PRIVATE) },
	{ FLAGS_DESC(MAP_SHARED) },
	{ FLAGS_DESC(MAP_PRIVATE | MAP_LOCKED) },
	{ FLAGS_DESC(MAP_SHARED | MAP_LOCKED) },
};

static void run_test(unsigned int i)
{
	int ret;
	void *p;
	struct tcase *tc = &tcases[i];

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	p = SAFE_MMAP(0, hpage_size, PROT_READ|PROT_WRITE, tc->flags, fd, 0);

	ret = mlock(p, hpage_size);
	if (ret) {
		tst_res(TFAIL|TERRNO, "mlock() failed (flags %s)", tc->flags_str);
		goto cleanup;
	}

	ret = munlock(p, hpage_size);
	if (ret)
		tst_res(TFAIL|TERRNO, "munlock() failed (flags %s)", tc->flags_str);
	else
		tst_res(TPASS, "mlock/munlock with %s hugetlb mmap", tc->flags_str);

cleanup:
	SAFE_MUNMAP(p, hpage_size);
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	struct rlimit limit_info;

	hpage_size = tst_get_hugepage_size();

	SAFE_GETRLIMIT(RLIMIT_MEMLOCK, &limit_info);
	if (limit_info.rlim_cur < hpage_size) {
		limit_info.rlim_max = hpage_size;
		limit_info.rlim_cur = hpage_size;
		SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &limit_info);
	}
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = run_test,
	.hugepages = {1, TST_NEEDS},
};
