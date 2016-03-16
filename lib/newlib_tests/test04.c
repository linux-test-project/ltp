/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 * Tests tst_run_as_child()
 */

#include "tst_test.h"

static void child_fn(unsigned int i)
{
	switch (i) {
	case 0:
		tst_res(TPASS, "PASSED message");
	break;
	case 1:
		tst_brk(TBROK, "BROKEN message");
	break;
	}
}

static void setup(void)
{
	tst_res(TINFO, "setup() executed by pid %i", getpid());
}

static void cleanup(void)
{
	tst_res(TINFO, "cleanup() executed by pid %i", getpid());
}

static void do_test(unsigned int i)
{
	if (SAFE_FORK() == 0)
		child_fn(i);
}

static struct tst_test test = {
	.tid = "test04",
	.tcnt = 2,
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
