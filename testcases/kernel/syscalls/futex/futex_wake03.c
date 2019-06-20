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
  * Block several processes on a mutex, then wake them up.
  */

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"
#include "futextest.h"

const char *TCID="futex_wake03";
const int TST_TOTAL=11;

static futex_t *futex;

static void do_child(void)
{
	futex_wait(futex, *futex, NULL, 0);
	exit(0);
}

static void do_wake(int nr_children)
{
	int res, i, cnt;

	res = futex_wake(futex, nr_children, 0);

	if (res != nr_children) {
		tst_resm(TFAIL,
		         "futex_wake() woken up %i children, expected %i",
		         res, nr_children);
		return;
	}

	for (cnt = 0, i = 0; i < 100000; i++) {
		while (waitpid(-1, &res, WNOHANG) > 0)
			cnt++;

		if (cnt == nr_children)
			break;

		usleep(100);
	}

	if (cnt != nr_children) {
		tst_resm(TFAIL, "reaped only %i childs, expected %i",
		         cnt, nr_children);
	} else {
		tst_resm(TPASS, "futex_wake() woken up %i childs", cnt);
	}
}

static void verify_futex_wake(void)
{
	int i, res;
	pid_t pids[55];

	for (i = 0; i < (int)ARRAY_SIZE(pids); i++) {
		pids[i] = tst_fork();

		switch (pids[i]) {
		case -1:
			tst_brkm(TBROK | TERRNO, NULL, "fork()");
		case 0:
			do_child();
		default:
		break;
		}
	}

	for (i = 0; i < (int)ARRAY_SIZE(pids); i++)
		tst_process_state_wait2(pids[i], 'S');

	for (i = 1; i <= 10; i++)
		do_wake(i);

	res = futex_wake(futex, 1, 0);

	if (res) {
		tst_resm(TFAIL, "futex_wake() woken up %u, none were waiting",
		         res);
	} else {
		tst_resm(TPASS, "futex_wake() woken up 0 children");
	}
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
		verify_futex_wake();

	tst_exit();
}
