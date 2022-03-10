// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Li Wang <liwang@redhat.com>
 */

/*
 * Tests .request_hugepages + .save_restore
 */

#include "tst_test.h"
#include "tst_hugepage.h"
#include "tst_sys_conf.h"

static void do_test(void) {

	unsigned long val, hpages;

	tst_res(TINFO, "tst_hugepages = %lu", tst_hugepages);
	SAFE_FILE_PRINTF("/proc/sys/kernel/numa_balancing", "1");

	hpages = test.request_hugepages;
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, hpages);
	else
		tst_res(TPASS, "test .needs_hugepges");

	hpages = tst_request_hugepages(3);
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, hpages);
	else
		tst_res(TPASS, "tst_request_hugepages");
}

static struct tst_test test = {
	.test_all = do_test,
	.request_hugepages = 2,
	.save_restore = (const struct tst_path_val[]) {
		{"!/proc/sys/kernel/numa_balancing", "0"},
		{}
	},
};
