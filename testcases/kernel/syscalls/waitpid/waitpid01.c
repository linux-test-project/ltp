/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    07/2001 John George
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Check that when a child kills itself by SIGALRM the waiting parent is
 * correctly notified.
 *
 * Fork a child that raises(SIGALRM), the parent checks that SIGALRM was
 * returned.
 */
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

static void run(void)
{
	pid_t pid, rpid;
	int status;

	pid = SAFE_FORK();
	if (!pid) {
		raise(SIGALRM);
		exit(0);
	}

	rpid = waitpid(pid, &status, 0);
	if (rpid < 0)
		tst_brk(TBROK | TERRNO, "waitpid() failed");

	if (rpid != pid) {
		tst_res(TFAIL, "waitpid() returned wrong pid %i, expected %i",
		        rpid, pid);
	} else {
		tst_res(TPASS, "waitpid() returned correct pid %i", pid);
	}

	if (!WIFSIGNALED(status)) {
		tst_res(TFAIL, "WIFSIGNALED() not set in status (%s)",
		        tst_strstatus(status));
		return;
	}

	tst_res(TPASS, "WIFSIGNALED() set in status");

	if (WTERMSIG(status) != SIGALRM) {
		tst_res(TFAIL, "WTERMSIG() != SIGALRM but %s",
		        tst_strsig(WTERMSIG(status)));
		return;
	}

	tst_res(TPASS, "WTERMSIG() == SIGALRM");
}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = run,
};
