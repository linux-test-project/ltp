// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_hugepage.h"

unsigned int tst_hugepages;

int tst_request_hugepages(int hpages)
{
	int val;
	long mem_avail, max_hpages;

	if (access(PATH_HUGEPAGES, F_OK)) {
		tst_hugepages = 0;
		goto out;
	}

	tst_hugepages = hpages;
	SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
	mem_avail = SAFE_READ_MEMINFO("MemFree:");
	max_hpages = mem_avail / SAFE_READ_MEMINFO("Hugepagesize:");

	if (hpages > max_hpages) {
		tst_res(TINFO, "Requested number(%d) of hugepages is too large, "
				"limiting to 80%% of the max hugepage count %ld",
				hpages, max_hpages);
		tst_hugepages = max_hpages * 0.8;

		if (tst_hugepages < 1)
			goto out;
	}

	tst_sys_conf_save("?/proc/sys/vm/nr_hugepages");
	SAFE_FILE_PRINTF(PATH_NR_HPAGES, "%d", tst_hugepages);
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%d", &val);
	if (val != tst_hugepages)
		tst_brk(TBROK, "nr_hugepages = %d, but expect %d", val, tst_hugepages);

	tst_res(TINFO, "%d hugepage(s) reserved", tst_hugepages);
out:
	return tst_hugepages;
}
