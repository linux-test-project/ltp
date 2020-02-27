// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Assert that tst_res() from child started by exec() is propagated to the main
 * test process.
 *
 * This test should be executed as:
 * $ PATH=$PATH:$PWD ./test_exec
 */

#define _GNU_SOURCE
#include <unistd.h>
#include "tst_test.h"

static void do_test(void)
{
	char *const argv[] = {"test_exec_child", NULL};
	char path[4096];

	if (tst_get_path("test_exec_child", path, sizeof(path)))
		tst_brk(TCONF, "Couldn't find test_exec_child in $PATH");

	execve(path, argv, environ);

	tst_res(TFAIL | TERRNO, "EXEC!");
}

static struct tst_test test = {
	.test_all = do_test,
	.child_needs_reinit = 1,
};
