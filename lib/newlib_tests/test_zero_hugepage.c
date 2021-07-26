// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*
 * Tests .request_hugepages = TST_NO_HUGEPAGES
 */

#include "tst_test.h"
#include "tst_hugepage.h"
#include "tst_sys_conf.h"

static void do_test(void)
{
	unsigned long val, hpages;

	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != 0)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect 0", val);
	else
		tst_res(TPASS, "test .request_hugepages = TST_NO_HUGEPAGES");

	hpages = tst_request_hugepages(3);
	SAFE_FILE_SCANF(PATH_NR_HPAGES, "%lu", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %lu, but expect %lu", val, hpages);
	else
		tst_res(TPASS, "tst_request_hugepages");
}

static struct tst_test test = {
	.test_all = do_test,
	.request_hugepages = TST_NO_HUGEPAGES,
};
