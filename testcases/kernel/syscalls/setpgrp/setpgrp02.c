// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) Linux Test Project, 2001-2016
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * Testcase to check the basic functionality of the setpgrp(2) syscall.
 */

#include <unistd.h>
#include "tst_test.h"

static void verify_setpgrp(void)
{
	if (!SAFE_FORK()) {
		int oldpgrp = getpgrp();

		TST_EXP_PASS(setpgrp());
		if (getpgrp() == oldpgrp)
			tst_res(TFAIL, "setpgrp() FAILED to set new group id");
		else
			tst_res(TPASS, "functionality is correct");
	}
}

static struct tst_test test = {
	.test_all = verify_setpgrp,
	.forks_child = 1
};
