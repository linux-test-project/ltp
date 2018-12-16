/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

	tst_res(TBROK | TERRNO, "EXEC!");
}

static struct tst_test test = {
	.test_all = do_test,
	.child_needs_reinit = 1,
};
