/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
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
  * Block on a futex and wait for wakeup.
  *
  * This tests uses shared memory page to store the mutex variable.
  */

#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"
#include "futextest.h"

const char *TCID="futex_wait02";
const int TST_TOTAL=1;

static futex_t *futex;

static void do_child(void)
{
	int ret;

	tst_process_state_wait2(getppid(), 'S');

	ret = futex_wake(futex, 1, 0);

	if (ret != 1)
		tst_brkm(TFAIL, NULL, "futex_wake() returned %i", ret);

	exit(TPASS);
}

static void verify_futex_wait(void)
{
	int res;
	int pid;

	pid = tst_fork();

	switch (pid) {
	case 0:
		do_child();
	break;
	case -1:
		tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
	break;
	default:
	break;
	}

	res = futex_wait(futex, *futex, NULL, 0);

	if (res) {
		tst_resm(TFAIL, "futex_wait() returned %i, errno %s",
		         res, tst_strerrno(errno));
	}

	SAFE_WAIT(NULL, &res);

	if (WIFEXITED(res) && WEXITSTATUS(res) == TPASS)
		tst_resm(TPASS, "futex_wait() woken up");
	else
		tst_resm(TFAIL, "child failed");
}

static void setup(void)
{
	futex = SAFE_MMAP(NULL, NULL, sizeof(*futex), PROT_READ | PROT_WRITE,
			  MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	*futex = FUTEX_INITIALIZER;
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_futex_wait();

	tst_exit();
}
