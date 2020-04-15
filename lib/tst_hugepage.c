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

unsigned long tst_request_hugepages(unsigned long hpages)
{
	unsigned long val, max_hpages;

	if (access(PATH_HUGEPAGES, F_OK)) {
		tst_hugepages = 0;
		goto out;
	}

	if (nr_opt)
		tst_hugepages = SAFE_STRTOL(nr_opt, 1, LONG_MAX);
	else
		tst_hugepages = hpages;

	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
	max_hpages = SAFE_READ_MEMINFO("MemFree:") / SAFE_READ_MEMINFO("Hugepagesize:");

	if (tst_hugepages > max_hpages) {
		tst_res(TINFO, "Requested number(%lu) of hugepages is too large, "
				"limiting to 80%% of the max hugepage count %lu",
				tst_hugepages, max_hpages);
		tst_hugepages = max_hpages * 0.8;

		if (tst_hugepages < 1)
			goto out;
	}

	tst_sys_conf_save("?/proc/sys/vm/nr_hugepages");
	SAFE_FILE_PRINTF(PATH_NR_HPAGES, "%lu", tst_hugepages);
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != tst_hugepages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, tst_hugepages);

	tst_res(TINFO, "%lu hugepage(s) reserved", tst_hugepages);
out:
	return tst_hugepages;
}
