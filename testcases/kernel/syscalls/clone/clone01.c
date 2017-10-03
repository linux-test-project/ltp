/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 *	This is a test for the clone(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 */

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"
#include "clone_platform.h"

static void setup(void);
static void cleanup(void);
static int do_child();

char *TCID = "clone01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	void *child_stack;
	int status, child_pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	child_stack = malloc(CHILD_STACK_SIZE);
	if (child_stack == NULL)
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");

	tst_count = 0;

	TEST(ltp_clone(SIGCHLD, do_child, NULL, CHILD_STACK_SIZE, child_stack));
	if (TEST_RETURN == -1)
		tst_resm(TFAIL | TTERRNO, "clone failed");

	child_pid = SAFE_WAIT(cleanup, &status);

	if (TEST_RETURN == child_pid)
		tst_resm(TPASS, "clone returned %ld", TEST_RETURN);
	else
		tst_resm(TFAIL, "clone returned %ld, wait returned %d",
			 TEST_RETURN, child_pid);

	free(child_stack);

	cleanup();

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
}

static int do_child(void)
{
	exit(0);
}
