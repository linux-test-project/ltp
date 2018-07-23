/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012-2018 Cyril Hrubis <chrubis@suse.cz>
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
 * wait401 - check that a call to wait4() correctly waits for a child
 *           process to exit
 */

#include <stdlib.h>
#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "tst_test.h"

static void run(void)
{
	pid_t pid;
	int status = 1;
	struct rusage rusage;

	pid = SAFE_FORK();
	if (!pid) {
		TST_PROCESS_STATE_WAIT(getppid(), 'S');
		exit(0);
	}

	TEST(wait4(pid, &status, 0, &rusage));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "wait4() failed");
		return;
	}

	if (TST_RET != pid) {
		tst_res(TFAIL, "waitpid() returned wrong pid %li, expected %i",
			TST_RET, pid);
	} else {
		tst_res(TPASS, "waitpid() returned correct pid %i", pid);
	}

	if (!WIFEXITED(status)) {
		tst_res(TFAIL, "WIFEXITED() not set in status (%s)",
		        tst_strstatus(status));
		return;
	}

	tst_res(TPASS, "WIFEXITED() is set in status");

	if (WEXITSTATUS(status))
		tst_res(TFAIL, "WEXITSTATUS() != 0 but %i", WEXITSTATUS(status));
	else
		tst_res(TPASS, "WEXITSTATUS() == 0");

}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = run,
};
