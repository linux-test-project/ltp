// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Li Wang <liwang@redhat.com>
 */

#include "tst_test.h"
#include "tst_kconfig.h"

static struct tst_kcmdline_var params[] = {
	TST_KCMDLINE_INIT("BOOT_IMAGE"),
	TST_KCMDLINE_INIT("root"),
	TST_KCMDLINE_INIT("foo")
};

static void do_test(void)
{
	int i, N;

	N = (int) ARRAY_SIZE(params);

	tst_kcmdline_parse(params, N);

	for (i = 0; i < N; i++) {
		if (params[i].found) {
			tst_res(TPASS, "params[%d] = {%s, %s}", i, params[i].key, params[i].value);
		} else {
			if (!strcmp(params[i].key, "foo"))
				tst_res(TPASS, "%s is not found in /proc/cmdline", params[i].key);
			else
				tst_res(TFAIL, "%s is not found in /proc/cmdline", params[i].key);
		}
	}
}

static struct tst_test test = {
	.test_all = do_test,
};
