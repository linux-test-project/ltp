// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that a raw :manpage:`syscall(2)` with the invalid system call
 * number -1 fails with ENOSYS.
 */

#define _GNU_SOURCE
#include <unistd.h>

#include "tst_test.h"

static void run(void)
{
	TST_EXP_FAIL2(syscall(-1), ENOSYS, "invalid syscall number -1");
}

static struct tst_test test = {
	.test_all = run,
};
