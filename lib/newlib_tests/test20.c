// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Li Wang <liwang@redhat.com>
 */

/*
 * Tests .hugepages + .save_restore
 */

#include "tst_test.h"
#include "tst_hugepage.h"
#include "tst_sys_conf.h"

static void do_test(void) {

	unsigned long val, hpages;

	tst_res(TINFO, "tst_hugepages = %lu", tst_hugepages);
	SAFE_FILE_PRINTF("/proc/sys/kernel/numa_balancing", "1");

	hpages = test.hugepages.number;
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, hpages);
	else
		tst_res(TPASS, "test .hugepges");

	struct tst_hugepage hp = { 1000000000000, TST_REQUEST };
	hpages = tst_reserve_hugepages(&hp);

	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, hpages);
	else
		tst_res(TPASS, "tst_reserve_hugepages");
}

static struct tst_test test = {
	.test_all = do_test,
	.hugepages = {2, TST_NEEDS},
	.save_restore = (const struct tst_path_val[]) {
		{"!/proc/sys/kernel/numa_balancing", "0"},
		{}
	},
};
