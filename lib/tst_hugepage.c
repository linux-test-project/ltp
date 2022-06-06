// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_hugepage.h"

unsigned long tst_hugepages;
char *nr_opt;
char *Hopt;

size_t tst_get_hugepage_size(void)
{
	if (access(PATH_HUGEPAGES, F_OK))
		return 0;

	return SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
}

unsigned long tst_reserve_hugepages(struct tst_hugepage *hp)
{
	unsigned long val, max_hpages;

	if (access(PATH_HUGEPAGES, F_OK)) {
		if (hp->policy == TST_NEEDS)
			tst_brk(TCONF, "hugetlbfs is not supported");
		tst_hugepages = 0;
		goto out;
	}

	if (nr_opt)
		tst_hugepages = SAFE_STRTOL(nr_opt, 1, LONG_MAX);
	else
		tst_hugepages = hp->number;

	if (hp->number == TST_NO_HUGEPAGES) {
		tst_hugepages = 0;
		goto set_hugepages;
	}

	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
	if (hp->policy == TST_NEEDS) {
		tst_hugepages += SAFE_READ_MEMINFO("HugePages_Total:");
		goto set_hugepages;
	}

	max_hpages = SAFE_READ_MEMINFO("MemFree:") / SAFE_READ_MEMINFO("Hugepagesize:");
	if (tst_hugepages > max_hpages) {
		tst_res(TINFO, "Requested number(%lu) of hugepages is too large, "
				"limiting to 80%% of the max hugepage count %lu",
				tst_hugepages, max_hpages);
		tst_hugepages = max_hpages * 0.8;

		if (tst_hugepages < 1)
			goto out;
	}

set_hugepages:
	tst_sys_conf_save("?/proc/sys/vm/nr_hugepages");
	SAFE_FILE_PRINTF(PATH_NR_HPAGES, "%lu", tst_hugepages);
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != tst_hugepages)
		tst_brk(TCONF, "nr_hugepages = %lu, but expect %lu. "
				"Not enough hugepages for testing.",
				val, tst_hugepages);

	if (hp->policy == TST_NEEDS) {
		unsigned long free_hpages = SAFE_READ_MEMINFO("HugePages_Free:");
		if (hp->number > free_hpages)
			tst_brk(TCONF, "free_hpages = %lu, but expect %lu. "
				"Not enough hugepages for testing.",
				free_hpages, hp->number);
	}

	tst_res(TINFO, "%lu hugepage(s) reserved", tst_hugepages);
out:
	return tst_hugepages;
}
